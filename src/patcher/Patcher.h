#pragma once
#include <QString>
#include <QUrl>
#include <QByteArray>

class Patcher
{
    public:
        Patcher(QString url);
        Patcher(QUrl url);

        void set_url(QString url);
        void set_url(QUrl url);
        QUrl get_url();

        void update_manifest(QString distribution_token,
                             QString filename = "patcher.xml");
        void parse_manifest(QByteArray manifest);

        QString get_launcher_version();
        QString get_account_server();
        QString get_client_agent();
        QString get_server_version();

    private:
        QUrl m_url;

        QString m_launcher_version;
        QString m_account_server;
        QString m_client_agent;
        QString m_server_version;
};
