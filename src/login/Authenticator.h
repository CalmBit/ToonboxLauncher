#pragma once
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

    LoginReply login(QString username, QString password,
                     QString distribution = "dev");

  private:
    QUrl m_login_endpoint;

    LoginReply parse_login_reply(QByteArray data);
};
