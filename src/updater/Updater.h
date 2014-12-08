#pragma once
#include "ManifestDirectory.h"
#include "ManifestFile.h"

#include "core/constants.h"

#include <exception>
#include <vector>
#include <queue>
#include <cstdint>

#include <QObject>
#include <QUrl>
#include <QString>
#include <QFile>
#include <QNetworkReply>
#include <QTime>
#include <QByteArray>
#include <QXmlStreamReader>

class DownloadError : public std::runtime_error
{
  public:
    DownloadError(int error_code, const QString &error_string) :
        std::runtime_error(error_string.toStdString()), m_error_code(error_code)
    {
    }

    int get_error_code()
    {
        return m_error_code;
    }

  private:
    int m_error_code;
};

class Updater : public QObject
{
    Q_OBJECT

  public:
    Updater(QUrl url);
    ~Updater();

    QString get_launcher_version();
    QString get_account_server();
    QString get_client_agent();
    QString get_server_version();

    std::vector<ManifestDirectory> get_directories();

    void update_manifest(const QString &filename = MANIFEST_FILENAME);
    bool update();

  signals:
    void download_error(int error_code, const QString &error_string);
    void download_progress(qint64 bytes_read, qint64 bytes_total, const QString &status);

  private slots:
    void readyRead();
    void finished();
    void downloadProgress(qint64 bytes_read, qint64 bytes_total);

  private:
    QUrl m_url;

    QString m_launcher_version;
    QString m_account_server;
    QString m_client_agent;
    QString m_server_version;

    std::vector<ManifestDirectory> m_directories;

    size_t m_update_file_number;
    size_t m_update_file_total;

    QFile *m_download_file;
    QNetworkReply *m_download_reply;
    QTime m_download_time;

    void add_directory(ManifestDirectory directory);

    void parse_manifest_data(const QByteArray &data);
    ManifestDirectory parse_manifest_directory(QXmlStreamReader &reader);
    ManifestFile parse_manifest_file(QXmlStreamReader &reader);

    void download_file(const QString &relative_path);
};
