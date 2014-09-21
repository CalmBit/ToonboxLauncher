#include "Updater.h"
#include "ManifestDirectory.h"
#include "ManifestFile.h"

#include "core/constants.h"
#include "core/localizer.h"

#include <vector>
#include <queue>
#include <sstream>
#include <cstdio>

#include <bzlib.h>

#include <QUrl>
#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QByteArray>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QCryptographicHash>
#include <QFuture>
#include <QtConcurrent>
#include <QFileInfo>

Updater::Updater(QUrl url) : QObject(), m_url(url), m_launcher_version(""),
    m_account_server(""), m_client_agent(""), m_server_version(""),
    m_update_file_total(0), m_update_file_number(0)
{
}

Updater::~Updater()
{
    m_event_loop.quit();
    if(m_download_output) {
        m_download_output->flush();
        m_download_output->close();
        delete m_download_output;
        m_download_output = nullptr;
    }
    if(m_download_reply) {
        delete m_download_reply;
        m_download_reply = nullptr;
    }
}

void Updater::set_url(QUrl url)
{
    m_url = url;
}

QUrl Updater::get_url()
{
    return m_url;
}

QString Updater::get_launcher_version()
{
    return m_launcher_version;
}

QString Updater::get_account_server()
{
    return m_account_server;
}

QString Updater::get_client_agent()
{
    return m_client_agent;
}

QString Updater::get_server_version()
{
    return m_server_version;
}

std::vector<ManifestDirectory> Updater::get_directories()
{
    return m_directories;
}

void Updater::add_directory(ManifestDirectory directory)
{
    m_directories.push_back(directory);
}

void Updater::update_manifest(QString filename)
{
    // Clear the data from the previous manifest:
    m_directories.clear();
    m_launcher_version = "";
    m_account_server = "";
    m_client_agent = "";
    m_server_version = "";

    QNetworkAccessManager manager;

    QNetworkRequest request(QUrl(m_url.toString() + filename));
    request.setHeader(QNetworkRequest::UserAgentHeader, USER_AGENT);
    QNetworkReply *reply = manager.get(request);

    // Pause execution until the network request is finished:
    QObject::connect(&manager, SIGNAL(finished(QNetworkReply *)),
                     &m_event_loop, SLOT(quit()));
    m_event_loop.exec();

    if(reply->error() == QNetworkReply::NoError) {
        this->parse_manifest(reply->readAll());
    }

    delete reply;
}

void Updater::parse_manifest(QByteArray data)
{
    QXmlStreamReader reader(data);

    while(!reader.atEnd() && !reader.hasError()) {
        if(reader.readNext() != QXmlStreamReader::StartElement) {
            continue;
        }

        QString name = reader.name().toString();
        if(name == "launcher-version") {
            if(reader.readNext() == QXmlStreamReader::Characters) {
                m_launcher_version = reader.text().toString();
            }
        } else if(name == "account-server") {
            if(reader.readNext() == QXmlStreamReader::Characters) {
                m_account_server = reader.text().toString();
            }
        } else if(name == "client-agent") {
            if(reader.readNext() == QXmlStreamReader::Characters) {
                m_client_agent = reader.text().toString();
            }
        } else if(name == "server-version") {
            if(reader.readNext() == QXmlStreamReader::Characters) {
                m_server_version = reader.text().toString();
            }
        } else if(name == "directory") {
            this->add_directory(this->parse_manifest_directory(reader));
        }
    }

    reader.clear();
}

ManifestDirectory Updater::parse_manifest_directory(QXmlStreamReader &reader)
{
    QXmlStreamAttributes attributes = reader.attributes();
    ManifestDirectory directory(attributes.value("name").toString());

    reader.readNext();

    while(reader.name() != "directory") {
        directory.add_file(this->parse_manifest_file(reader));
    }

    return directory;
}

ManifestFile Updater::parse_manifest_file(QXmlStreamReader &reader)
{
    QXmlStreamAttributes attributes = reader.attributes();
    ManifestFile file(attributes.value("name").toString(), 0, "");

    do {
        if(reader.readNext() != QXmlStreamReader::StartElement) {
            continue;
        }

        QString name = reader.name().toString();
        if(name == "size") {
            if(reader.readNext() == QXmlStreamReader::Characters) {
                file.set_size(reader.text().toLongLong());
            }
        } else if(name == "hash") {
            if(reader.readNext() == QXmlStreamReader::Characters) {
                file.set_hash(reader.text().toUtf8());
            }
        }
    } while(reader.name() != "file");

    reader.readNext();

    return file;
}

void Updater::update_files()
{
    // Build a queue of files to be updated:
    std::queue<QString> file_queue;

    QDir directory = QDir::current();

    for(auto it = m_directories.begin(); it != m_directories.end(); ++it) {
        QString directory_name = it->get_name();
        if(!directory_name.isEmpty()) {
            // Create the directory if it doesn't exist:
            if(!directory.exists(directory_name)) {
                directory.mkdir(directory_name);
            }

            // Move into the directory:
            directory.cd(directory_name);
        }

        std::vector<ManifestFile> files = it->get_files();
        for(auto it2 = files.begin(); it2 != files.end(); ++it2) {
            QString file_name = it2->get_name();
            QString absolute_path = directory.absoluteFilePath(file_name);
            QString relative_path = QDir::current().relativeFilePath(absolute_path);

            if(!directory.exists(file_name)) {
                file_queue.push(relative_path);
                continue;
            }

            QFile file(absolute_path);
            if(!file.open(QIODevice::ReadOnly)) {
                file_queue.push(relative_path);
                continue;
            }

            qint64 size = file.size();
            QByteArray hash = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5);
            if((size != it2->get_size()) || (hash.toHex() != it2->get_hash())) {
                file_queue.push(relative_path);
            }

            file.close();
        }

        if(!directory_name.isEmpty()) {
            // Move out of the directory:
            directory.cdUp();
        }
    }

    m_update_file_total = file_queue.size();
    for(m_update_file_number = 1; !file_queue.empty(); m_update_file_number++) {
        QString output_path = file_queue.front();
        QString archive_path = output_path.section(".", 0, 0) + ".bz2";
        try {
            this->download_file(archive_path);
        } catch(DownloadError &e) {
            emit download_error(ERROR_CODE_DOWNLOADING, e.what());
            break;
        }
        try {
            QObject::connect(this, SIGNAL(extract_finished()),
                             &m_event_loop, SLOT(quit()));
            QFuture<void> future = QtConcurrent::run(
                this, &Updater::extract_file, archive_path, output_path);
            m_event_loop.exec();
            future.waitForFinished();
        } catch(ExtractionThreadError &e) {
            emit download_error(ERROR_CODE_EXTRACTING, e.error().what());
            break;
        }

        file_queue.pop();
    }
}

void Updater::download_file(QString filepath)
{
    m_download_output = new QFile(filepath);
    if(!m_download_output->open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        delete m_download_output;
        m_download_output = nullptr;

        QString filename = QFileInfo(filepath).fileName();
        throw DownloadError(ERROR_WRITE.arg(filename).toStdString());
    }

    QNetworkAccessManager manager;

    QNetworkRequest request(QUrl(m_url.toString() + filepath));
    request.setHeader(QNetworkRequest::UserAgentHeader, USER_AGENT);
    m_download_reply = manager.get(request);
    m_download_time.start();

    // Pause execution until the download is finished:
    QObject::connect(m_download_reply, SIGNAL(finished()),
                     &m_event_loop, SLOT(quit()));
    QObject::connect(m_download_reply, SIGNAL(readyRead()),
                     this, SLOT(download_ready_read()));
    QObject::connect(m_download_reply, SIGNAL(downloadProgress(qint64, qint64)),
                     this, SLOT(download_progress(qint64, qint64)));
    m_event_loop.exec();

    m_download_output->flush();
    m_download_output->close();
    delete m_download_output;
    m_download_output = nullptr;
    delete m_download_reply;
    m_download_reply = nullptr;
}

void Updater::download_ready_read()
{
    if(m_download_output) {
        m_download_output->write(m_download_reply->readAll());
    }
}

void Updater::download_progress(qint64 bytes_read, qint64 bytes_total)
{
    // Choose the total unit:
    double read = bytes_read;
    double total = bytes_total;
    QString unit;
    if(bytes_total < 1024) {
        unit = "B";
    } else if(bytes_total < (1024*1024)) {
        read /= 1024;
        total /= 1024;
        unit = "kB";
    } else {
        read /= 1024 * 1024;
        total /= 1024 * 1024;
        unit = "MB";
    }

    // Round the values to the nearest tenth:
    read = floor((read*10.0) + 0.5) / 10.0;
    total = floor((total*10.0) + 0.5) / 10.0;

    // Calculate the download speed, and choose the speed unit:
    double speed = (bytes_read*1000.0) / m_download_time.elapsed();
    QString speed_unit;
    if(speed < 1024) {
        speed_unit = "B/s";
    } else if(speed < (1024*1024)) {
        speed /= 1024;
        speed_unit = "kB/s";
    } else {
        speed /= 1024 * 1024;
        speed_unit = "MB/s";
    }

    // Round the value to the nearest tenth:
    speed = floor((speed*10.0) + 0.5) / 10.0;

    // Emit a status update signal:
    emit this->download_progressed(bytes_read, bytes_total,
        GUI_DOWNLOAD_WAITING.arg(
            QString::number(m_update_file_number),
            QString::number(m_update_file_total), QString::number(read),
            QString::number(total), unit, QString::number(speed), speed_unit));
}

void Updater::extract_file(QString archive_path, QString output_path)
{
    FILE *f = fopen(archive_path.toStdString().c_str(), "rb");
    if(f == NULL) {
        QString filename = QFileInfo(archive_path).fileName();
        try {
            throw ExtractionError(ERROR_READ.arg(filename).toStdString());
        } catch(ExtractionError &e) {
            emit this->extract_finished();
            throw ExtractionThreadError(e);
        }
    }

    int bzerror;
    BZFILE *archive_file = BZ2_bzReadOpen(&bzerror, f, 0, 0, NULL, 0);
    if(bzerror != BZ_OK) {
        QString filename = QFileInfo(archive_path).fileName();
        try {
            throw ExtractionError(ERROR_DECOMPRESS.arg(filename).toStdString());
        } catch(ExtractionError &e) {
            emit this->extract_finished();
            throw ExtractionThreadError(e);
        }
    }

    FILE *output_file = fopen(output_path.toStdString().c_str(), "wb");
    if(output_file == NULL) {
        QString filename = QFileInfo(output_path).fileName();
        try {
            throw ExtractionError(ERROR_WRITE.arg(filename).toStdString());
        } catch(ExtractionError &e) {
            emit this->extract_finished();
            throw ExtractionThreadError(e);
        }
    }

    char buffer[4096];
    while(bzerror == BZ_OK) {
        int nread = BZ2_bzRead(&bzerror, archive_file, buffer, sizeof(buffer));
        if((bzerror == BZ_OK) || bzerror == BZ_STREAM_END) {
            size_t nwritten = fwrite(buffer, 1, nread, output_file);
            if(nwritten != (size_t)nread) {
                QString filename = QFileInfo(archive_path).fileName();
                try {
                    throw ExtractionError(ERROR_DECOMPRESS.arg(filename).toStdString());
                } catch(ExtractionError &e) {
                    emit this->extract_finished();
                    throw ExtractionThreadError(e);
                }
            }
        }
    }

    if(bzerror != BZ_STREAM_END) {
        QString filename = QFileInfo(archive_path).fileName();
        try {
            throw ExtractionError(ERROR_DECOMPRESS.arg(filename).toStdString());
        } catch(ExtractionError &e) {
            emit this->extract_finished();
            throw ExtractionThreadError(e);
        }
    }

    BZ2_bzReadClose(&bzerror, archive_file);
    fclose(output_file);
    QFile::remove(archive_path);

    emit this->extract_finished();
}
