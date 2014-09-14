#include "Patcher.h"

#include "core/constants.h"

#include <QString>
#include <QUrl>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QObject>
#include <QByteArray>
#include <QXmlStreamReader>

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

void Patcher::update_manifest(QString distribution_token, QString filename)
{
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

    if(reply->error() == QNetworkReply::NoError)
    {
        this->parse_manifest(reply->readAll());
    }

    delete reply;
}

void Patcher::parse_manifest(QByteArray manifest)
{
    QXmlStreamReader reader(manifest);

    while(!reader.atEnd() && !reader.hasError())
    {
        if(reader.readNext() != QXmlStreamReader::StartElement)
        {
            continue;
        }

        QString name = reader.name().toString();
        if((name == "patcher") || (reader.readNext() != QXmlStreamReader::Characters))
        {
            continue;
        }

        if(name == "launcher-version")
        {
            m_launcher_version = reader.text().toString();
        }
        else if(name == "account-server")
        {
            m_account_server = reader.text().toString();
        }
        else if(name == "client-agent")
        {
            m_client_agent = reader.text().toString();
        }
        else if(name == "server-version")
        {
            m_server_version = reader.text().toString();
        }
    }

    reader.clear();
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
