#include "Authenticator.h"

#include "core/constants.h"
#include "core/localizer.h"

#include <QUrl>
#include <QString>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QByteArray>
#include <QNetworkReply>
#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>

Authenticator::Authenticator(QUrl login_endpoint) :
    m_login_endpoint(login_endpoint)
{
}

void Authenticator::set_login_endpoint(QUrl login_endpoint)
{
    m_login_endpoint = login_endpoint;
}

QUrl Authenticator::get_login_endpoint()
{
    return m_login_endpoint;
}

LoginReply Authenticator::login(const QString &username, const QString &password,
                                const QString &distribution)
{
    QEventLoop event_loop;
    QNetworkAccessManager network_access_manager;

    QNetworkRequest request(m_login_endpoint);
    request.setHeader(QNetworkRequest::UserAgentHeader, USER_AGENT);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/x-www-form-urlencoded");
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

    if(reply->error() == QNetworkReply::NoError) {
        login_reply = this->parse_login_reply(reply->readAll());
    } else {
        login_reply.error_code = ERROR_CODE_NO_CONNECTION;
        login_reply.response = ERROR_NO_CONNECTION;
    }

    delete reply;

    return login_reply;
}

LoginReply Authenticator::parse_login_reply(const QByteArray &data)
{
    LoginReply login_reply;
    login_reply.success = false;
    login_reply.error_code = ERROR_CODE_INVALID_RESPONSE;
    login_reply.response = ERROR_INVALID_RESPONSE;

    QJsonDocument document = QJsonDocument::fromJson(data);
    QJsonObject object = document.object();
    if(!object.contains("success")) {
        return login_reply;
    }

    login_reply.success = object["success"].toBool();
    if(login_reply.success) {
        login_reply.error_code = 0;
    } else if(object.contains("errorCode")) {
        login_reply.error_code = object["errorCode"].toInt();
    } else {
        return login_reply;
    }

    if(login_reply.success && object.contains("token")) {
        login_reply.response = object["token"].toString();
    } else if(!login_reply.success && object.contains("reason")) {
        login_reply.response = object["reason"].toString();
    }

    return login_reply;
}
