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
#include <QJsonDocument>
#include <QJsonObject>

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

LoginReply Authenticator::login(QString username, QString password, QString distribution)
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

    LoginReply login_reply;
    login_reply.success = false;

    if(reply->error() == QNetworkReply::NoError)
    {
        login_reply = this->parse_login_reply(reply->readAll());
    }
    else
    {
        login_reply.error_code = 900;
        login_reply.response = "Couldn't connect to the account server.";
    }

    delete reply;

    return login_reply;
}

LoginReply Authenticator::parse_login_reply(QByteArray reply)
{
    LoginReply login_reply;
    login_reply.success = false;
    login_reply.response = "Couldn't connect to the account server.";

    QJsonDocument document = QJsonDocument::fromJson(reply);
    QJsonObject object = document.object();
    if(!object.contains("success"))
    {
        login_reply.error_code = 901;
        return login_reply;
    }

    login_reply.success = object["success"].toBool();
    if(login_reply.success)
    {
        login_reply.error_code = 0;
    }
    else if(object.contains("errorCode"))
    {
        login_reply.error_code = object["errorCode"].toInt();
    }
    else
    {
        login_reply.error_code = 902;
        return login_reply;
    }

    if(login_reply.success && object.contains("token"))
    {
        login_reply.response = object["token"].toString();
    }
    else if(!login_reply.success && object.contains("reason"))
    {
        login_reply.response = object["reason"].toString();
    }
    else
    {
        login_reply.error_code = 903;
    }

    return login_reply;
}
