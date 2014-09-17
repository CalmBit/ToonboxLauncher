#include "Patcher.h"
#include "PatchFile.h"
#include "PatchDirectory.h"

#include "core/constants.h"

#include <vector>

#include <QString>
#include <QUrl>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QObject>
#include <QByteArray>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>

Patcher::Patcher(QString url) : m_url(url), m_launcher_version(""),
    m_account_server(""), m_client_agent(""), m_server_version("")
{
}

Patcher::Patcher(QUrl url) : m_url(url), m_launcher_version(""),
    m_account_server(""), m_client_agent(""), m_server_version("")
{
}

void Patcher::set_url(QString url)
{
    m_url = QUrl(url);
}

void Patcher::set_url(QUrl url)
{
    m_url = url;
}

QUrl Patcher::get_url()
{
    return m_url;
}

void Patcher::clear_manifest()
{
    m_directories.clear();

    m_launcher_version = "";
    m_account_server = "";
    m_client_agent = "";
    m_server_version = "";
}

void Patcher::update_manifest(QString distribution_token, QString filename)
{
    this->clear_manifest();

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

void Patcher::parse_manifest(QByteArray manifest)
{
    QXmlStreamReader reader(manifest);

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
            this->add_directory(this->parse_directory(reader));
        }
    }

    reader.clear();
}

PatchDirectory Patcher::parse_directory(QXmlStreamReader &reader)
{
    QXmlStreamAttributes attributes = reader.attributes();
    PatchDirectory directory(attributes.value("name").toString());

    reader.readNext();

    while(reader.name() != "directory") {
        directory.add_file(this->parse_file(reader));
    }

    return directory;
}

PatchFile Patcher::parse_file(QXmlStreamReader &reader)
{
    QXmlStreamAttributes attributes = reader.attributes();
    PatchFile file(attributes.value("name").toString(), 0, "");

    do {
        if(reader.readNext() != QXmlStreamReader::StartElement) {
            continue;
        }

        QString name = reader.name().toString();
        if(name == "size") {
            if(reader.readNext() == QXmlStreamReader::Characters) {
                file.set_size(reader.text().toULong());
            }
        } else if(name == "hash") {
            if(reader.readNext() == QXmlStreamReader::Characters) {
                file.set_hash(reader.text().toString());
            }
        }
    } while(reader.name() != "file");

    reader.readNext();

    return file;
}

void Patcher::add_directory(PatchDirectory directory)
{
    m_directories.push_back(directory);
}

std::vector<PatchDirectory> Patcher::get_directories()
{
    return m_directories;
}

QString Patcher::get_launcher_version()
{
    return m_launcher_version;
}

QString Patcher::get_account_server()
{
    return m_account_server;
}

QString Patcher::get_client_agent()
{
    return m_client_agent;
}

QString Patcher::get_server_version()
{
    return m_server_version;
}
