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
        Authenticator(QString url);
        Authenticator(QUrl url);

        void set_url(QString url);
        void set_url(QUrl url);
        QUrl get_url();

        LoginReply login(QString username, QString password,
                         QString distribution = "dev");
        LoginReply parse_login_reply(QByteArray reply);

    private:
        QUrl m_url;
};
