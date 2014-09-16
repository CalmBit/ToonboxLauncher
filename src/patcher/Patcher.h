#pragma once
#include "PatchFile.h"
#include "PatchDirectory.h"

#include <vector>

#include <QString>
#include <QUrl>
#include <QByteArray>
#include <QXmlStreamReader>

class Patcher
{
    public:
        Patcher(QString url);
        Patcher(QUrl url);

        void set_url(QString url);
        void set_url(QUrl url);
        QUrl get_url();

        void clear_manifest();
        void update_manifest(QString distribution_token,
                             QString filename = "patcher.xml");
        void parse_manifest(QByteArray manifest);
        PatchDirectory parse_directory(QXmlStreamReader &reader);
        PatchFile parse_file(QXmlStreamReader &reader);

        void add_directory(PatchDirectory directory);
        std::vector<PatchDirectory> get_directories();

        QString get_launcher_version();
        QString get_account_server();
        QString get_client_agent();
        QString get_server_version();

    private:
        QUrl m_url;

        std::vector<PatchDirectory> m_directories;

        QString m_launcher_version;
        QString m_account_server;
        QString m_client_agent;
        QString m_server_version;
};
