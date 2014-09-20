#include "Updater.h"
#include "ManifestDirectory.h"
#include "ManifestFile.h"

#include <vector>
#include <cmath>

#include <QUrl>
#include <QString>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QObject>
#include <QByteArray>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QFile>
#include <QIODevice>
#include <QtGlobal>

Updater::Updater(QUrl url) : QObject(), m_url(url), m_launcher_version(""),
    m_account_server(""), m_client_agent(""), m_server_version("")
{
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

void Updater::update_manifest(QString distribution_token, QString filename)
{
    // Clear the data from the previous manifest:
    m_directories.clear();
    m_launcher_version = "";
    m_account_server = "";
    m_client_agent = "";
    m_server_version = "";

    QEventLoop event_loop;
    QNetworkAccessManager network_access_manager;

    QUrl url(m_url.toString() + "/" + distribution_token + "/" + filename);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, USER_AGENT);
    QNetworkReply *reply = network_access_manager.get(request);

    // Pause execution until the network request is finished:
    QObject::connect(&network_access_manager, SIGNAL(finished(QNetworkReply *)),
                     &event_loop, SLOT(quit()));
    event_loop.exec();

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
                file.set_hash(reader.text().toUtf8().toHex());
            }
        }
    } while(reader.name() != "file");

    reader.readNext();

    return file;
}

void Updater::download_file(QString distribution_token, QString relative_path)
{
    QString archive_path = relative_path.section(".", 0, 0) + ".bz2";

    m_download_output = new QFile(archive_path);
    if(!m_download_output->open(QIODevice::WriteOnly)) {
        delete m_download_output;
        m_download_output = nullptr;
        return;
    }

    QEventLoop event_loop;
    QNetworkAccessManager network_access_manager;

    QUrl url(m_url.toString() + "/" + distribution_token + "/" + archive_path);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, USER_AGENT);
    m_download_reply = network_access_manager.get(request);
    m_download_time.start();

    // Pause execution until the download is finished:
    QObject::connect(m_download_reply, SIGNAL(finished()),
                     &event_loop, SLOT(quit()));
    QObject::connect(m_download_reply, SIGNAL(readyRead()),
                     this, SLOT(download_ready_read()));
    QObject::connect(m_download_reply, SIGNAL(downloadProgress(qint64, qint64)),
                     this, SLOT(download_progress(qint64, qint64)));
    event_loop.exec();

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
    QString status = "[%1/%2%3 @ %4%5]";

    // Choose the total unit:
    double read = bytes_read;
    double total = bytes_total;
    QString total_unit;
    if(bytes_total < 1024) {
        total_unit = "B";
    } else if(bytes_total < (1024*1024)) {
        read /= 1024;
        total /= 1024;
        total_unit = "kB";
    } else {
        read /= 1024 * 1024;
        total /= 1024 * 1024;
        total_unit = "MB";
    }

    // Round:
    read = floor((read*10.0) + 0.5) / 10.0;
    total = floor((total*10.0) + 0.05) / 10.0;

    // Calculate the download speed:
    double speed = (bytes_read*1000.0) / m_download_time.elapsed();

    // Choose the speed unit:
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

    // Round:
    speed = floor((speed*10.0) + 0.5) / 10.0;

    // Emit the status update signal:
    emit this->download_status(bytes_read, bytes_total,
        status.arg(QString::number(read), QString::number(total), total_unit,
                   QString::number(speed), speed_unit));
}
