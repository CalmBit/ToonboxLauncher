#include "Updater.h"
#include "ManifestDirectory.h"
#include "ManifestFile.h"

#include "core/constants.h"
#include "core/localizer.h"

#include <vector>
#include <queue>
#include <cstdio>

#include <bzlib.h>

#include <QUrl>
#include <QObject>
#include <QString>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QByteArray>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QtGlobal>
#include <QCryptographicHash>
#include <QFuture>
#include <QtConcurrent>
#include <QFileInfo>
#include <QIODevice>

Updater::Updater(QUrl url) : QObject(), m_url(url), m_launcher_version(""),
    m_account_server(""), m_client_agent(""), m_server_version(""), m_update_file_number(0),
    m_update_file_total(0)
{
}

Updater::~Updater()
{
    if(m_download_reply) {
        delete m_download_reply;
        m_download_reply = nullptr;
    }
    if(m_download_file) {
        m_download_file->close();
        delete m_download_file;
        m_download_file = nullptr;
    }
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

void Updater::update_manifest(const QString &filename)
{
    // Clear the data from the previous manifest:
    m_directories.clear();
    m_server_version = "";
    m_client_agent = "";
    m_account_server = "";
    m_launcher_version = "";

    QNetworkRequest request(QUrl(m_url.toString() + filename));
    request.setHeader(QNetworkRequest::UserAgentHeader, USER_AGENT);
    QNetworkAccessManager access_manager;
    QNetworkReply *reply = access_manager.get(request);

    // Block until the network request is finished:
    QEventLoop event_loop;
    QObject::connect(&access_manager, SIGNAL(finished(QNetworkReply *)),
                     &event_loop, SLOT(quit()));
    event_loop.exec();

    if(reply->error() == QNetworkReply::NoError) {
        this->parse_manifest_data(reply->readAll());
    }

    delete reply;
}

void Updater::parse_manifest_data(const QByteArray &data)
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

bool Updater::update()
{
    // Build a queue of files that need to be updated:
    std::queue<QString> file_queue;

    QDir directory = QDir::current();
    for(auto it = m_directories.begin(); it != m_directories.end(); ++it) {
        QString directory_name = it->get_name();
        if(!directory_name.isEmpty()) {
            // Create this directory if it doesn't already exist:
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
            QCryptographicHash hash(QCryptographicHash::Md5);
            qint64 bytes_read;
            char buffer[8192];
            while((bytes_read = file.read(buffer, 8192)) > 0) {
                hash.addData(buffer, bytes_read);
            }
            if((size != it2->get_size()) || (hash.result().toHex() != it2->get_hash())) {
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
        QString relative_path = file_queue.front();
        QFileInfo file_info(relative_path);
        QString archive_path = file_info.path() + "/" + file_info.completeBaseName() + ".bz2";

        try {
            this->download_file(archive_path);
        } catch(DownloadError &e) {
            emit download_error(e.get_error_code(), e.what());
            return false;
        }

        try {
            QFuture<void> future = QtConcurrent::run(
                this, &Updater::extract_file, archive_path, relative_path);
            QEventLoop event_loop;
            QObject::connect(this, SIGNAL(extract_finished()),
                             &event_loop, SLOT(quit()));
            event_loop.exec();
            future.waitForFinished();
        } catch(DownloadThreadError &e) {
            emit download_error(e.error().get_error_code(), e.error().what());
            return false;
        }

        file_queue.pop();
    }

    return true;
}

void Updater::download_file(const QString &relative_path)
{
    m_download_file = new QFile(relative_path);
    if(!m_download_file->open(QIODevice::WriteOnly)) {
        delete m_download_file;
        m_download_file = nullptr;

        QString file_name = QFileInfo(relative_path).fileName();
        throw DownloadError(ERROR_CODE_WRITE, ERROR_WRITE.arg(file_name));
    }

    QNetworkRequest request(QUrl(m_url.toString() + relative_path));
    request.setHeader(QNetworkRequest::UserAgentHeader, USER_AGENT);
    QNetworkAccessManager access_manager;
    m_download_reply = access_manager.get(request);

    m_download_time.start();

    QObject::connect(m_download_reply, SIGNAL(readyRead()),
                     this, SLOT(readyRead()));
    QObject::connect(m_download_reply, SIGNAL(finished()),
                     this, SLOT(finished()));
    QObject::connect(m_download_reply, SIGNAL(downloadProgress(qint64, qint64)),
                     this, SLOT(downloadProgress(qint64, qint64)));

    // Block until the network request is finished:
    QEventLoop event_loop;
    QObject::connect(m_download_reply, SIGNAL(finished()),
                     &event_loop, SLOT(quit()));
    event_loop.exec();

    if(m_download_reply) {
        delete m_download_reply;
        m_download_reply = nullptr;
    }
}

void Updater::readyRead()
{
    if(m_download_file && m_download_reply) {
        m_download_file->write(m_download_reply->readAll());
    }
}

void Updater::finished()
{
    if(m_download_file) {
        m_download_file->close();
        delete m_download_file;
        m_download_file = nullptr;
    }
}

void Updater::downloadProgress(qint64 bytes_read, qint64 bytes_total)
{
    // Choose the unit:
    QString unit = "B";
    double read = bytes_read;
    double total = bytes_total;
    if(bytes_total >= 1024) {
        if(bytes_total < (1024*1024)) {
            read /= 1024;
            total /= 1024;
            unit = "kB";
        } else {
            read /= 1024 * 1024;
            total /= 1024 * 1024;
            unit = "MB";
        }
    }

    // Round the values to the nearest tenth:
    read = floor((read*10.0) + 0.5) / 10.0;
    total = floor((total*10.0) + 0.5) / 10.0;

    // Calculate the download speed:
    double speed = (bytes_read*1000.0) / m_download_time.elapsed();

    // Choose the speed unit:
    QString speed_unit = "B/s";
    if(speed >= 1024) {
        if(speed < (1024*1024)) {
            speed /= 1024;
            speed_unit = "kB/s";
        } else {
            speed /= 1024 * 1024;
            speed_unit = "MB/s";
        }
    }

    // Round the value to the nearest tenth:
    speed = floor((speed*10.0) + 0.5) / 10.0;

    emit this->download_progress(bytes_read, bytes_total,
        GUI_DOWNLOAD_WAITING.arg(
            QString::number(m_update_file_number),
            QString::number(m_update_file_total), QString::number(read),
            QString::number(total), unit, QString::number(speed), speed_unit));
}

void Updater::extract_file(const QString &archive_path, const QString &output_path)
{
    FILE *f = fopen(archive_path.toStdString().c_str(), "rb");
    if(f == NULL) {
        emit this->extract_finished();
        try {
            QString file_name = QFileInfo(archive_path).fileName();
            throw DownloadError(ERROR_CODE_READ, ERROR_READ.arg(file_name));
        } catch(DownloadError &e) {
            throw DownloadThreadError(e);
        }
    }

    int bzerror;
    BZFILE *archive_file = BZ2_bzReadOpen(&bzerror, f, 0, 0, NULL, 0);
    if(bzerror != BZ_OK) {
        emit this->extract_finished();
        try {
            QString file_name = QFileInfo(archive_path).fileName();
            throw DownloadError(ERROR_CODE_EXTRACT, ERROR_EXTRACT.arg(file_name));
        } catch(DownloadError &e) {
            throw DownloadThreadError(e);
        }
    }

    FILE *output_file = fopen(output_path.toStdString().c_str(), "wb");
    if(output_file == NULL) {
        emit this->extract_finished();
        try {
            QString file_name = QFileInfo(output_path).fileName();
            throw DownloadError(ERROR_CODE_WRITE, ERROR_WRITE.arg(file_name));
        } catch(DownloadError &e) {
            throw DownloadThreadError(e);
        }
    }

    char buffer[4096];
    while(bzerror == BZ_OK) {
        int nread = BZ2_bzRead(&bzerror, archive_file, buffer, sizeof(buffer));
        if((bzerror == BZ_OK) || bzerror == BZ_STREAM_END) {
            size_t nwritten = fwrite(buffer, 1, nread, output_file);
            if(nwritten != (size_t)nread) {
                emit this->extract_finished();
                try {
                    QString file_name = QFileInfo(archive_path).fileName();
                    throw DownloadError(ERROR_CODE_EXTRACT, ERROR_EXTRACT.arg(file_name));
                } catch(DownloadError &e) {
                    throw DownloadThreadError(e);
                }
            }
        }
    }

    if(bzerror != BZ_STREAM_END) {
        emit this->extract_finished();
        try {
            QString file_name = QFileInfo(archive_path).fileName();
            throw DownloadError(ERROR_CODE_EXTRACT, ERROR_EXTRACT.arg(file_name));
        } catch(DownloadError &e) {
            throw DownloadThreadError(e);
        }
    }

    BZ2_bzReadClose(&bzerror, archive_file);
    fclose(f);
    fclose(output_file);
    QFile::remove(archive_path);

    emit this->extract_finished();
}
