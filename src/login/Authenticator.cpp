#include "Authenticator.h"

#include "core/constants.h"

#include <QString>
#include <QUrl>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QByteArray>
#include <QNetworkReply>
#include <QObject>

Authenticator::Authenticator(QString url) : m_url(url)
{
}

Authenticator::Authenticator(QUrl url) : m_url(url)
{
}

void Authenticator::set_url(QString url)
{
    m_url = QUrl(url);
}

void Authenticator::set_url(QUrl url)
{
    m_url = url;
}

QUrl Authenticator::get_url()
{
    return m_url;
}

QString Authenticator::login(QString username, QString password,
                             QString distribution)
{
    QEventLoop event_loop;
    QNetworkAccessManager network_access_manager;

    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::UserAgentHeader, USER_AGENT);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QByteArray parameters;
    parameters.append("n=" + username);
    parameters.append("&p=" + password);
    parameters.append("&dist=" + distribution);
    QNetworkReply *reply = network_access_manager.post(request, parameters);

    // Pause execution until the network request is finished:
    QObject::connect(&network_access_manager, SIGNAL(finished(QNetworkReply *)),
                     &event_loop, SLOT(quit()));
    event_loop.exec();

    QByteArray response;
    if(reply->error() == QNetworkReply::NoError)
    {
        response = reply->readAll();
    }

    delete reply;

    return QString(response);
}
