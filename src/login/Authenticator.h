#pragma once
#include <QString>
#include <QUrl>

class Authenticator
{
    public:
        Authenticator(QString url);
        Authenticator(QUrl url);

        void set_url(QString url);
        void set_url(QUrl url);
        QUrl get_url();

        QString login(QString username, QString password,
                      QString distribution = "dev");

    private:
        QUrl m_url;
};
