#pragma once
#include "core/constants.h"

#include <QString>
#include <QUrl>
#include <QByteArray>

struct LoginReply
{
    bool success;
    int error_code;
    QString response;
};

class Authenticator
{
  public:
    Authenticator(QUrl login_endpoint);

    void set_login_endpoint(QUrl login_endpoint);
    QUrl get_login_endpoint();

    LoginReply login(const QString &username, const QString &password,
                     const QString &distribution = DISTRIBUTION);

  private:
    QUrl m_login_endpoint;

    LoginReply parse_login_reply(const QByteArray &data);
};
