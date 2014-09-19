#include "Updater.h"
#include "ManifestDirectory.h"
#include "ManifestFile.h"

#include <vector>

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

Updater::Updater(QUrl url) : m_url(url), m_launcher_version(""),
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
