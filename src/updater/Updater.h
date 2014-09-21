#pragma once
#include "ManifestDirectory.h"
#include "ManifestFile.h"

#include "core/constants.h"

#include <exception>
#include <string>
#include <vector>

#include <QObject>
#include <QUrl>
#include <QString>
#include <QtGlobal>
#include <QEventLoop>
#include <QTime>
#include <QNetworkReply>
#include <QFile>
#include <QByteArray>
#include <QXmlStreamReader>

class DownloadError : public std::runtime_error
{
  public:
    DownloadError(const std::string &what) : std::runtime_error(what)
    {
    }
};

class ExtractionError : public std::runtime_error
{
  public:
    ExtractionError(const std::string &what) : std::runtime_error(what)
    {
    }
};

class Updater : public QObject
{
    Q_OBJECT

  public:
    Updater(QUrl url);
    ~Updater();

    void set_url(QUrl url);
    QUrl get_url();

    QString get_launcher_version();
    QString get_account_server();
    QString get_client_agent();
    QString get_server_version();

    std::vector<ManifestDirectory> get_directories();

    void update_manifest(QString filename = MANIFEST_FILENAME);
    void update_files();
    void download_file(QString filepath);
    void extract_file(QString archive_path, QString output_path);

  signals:
    void download_progressed(qint64 bytes_read, qint64 bytes_total, QString status);
    void download_error(int error_code, QString reason);

  private slots:
    void download_ready_read();
    void download_progress(qint64 bytes_read, qint64 bytes_total);

  private:
    QUrl m_url;

    QString m_launcher_version;
    QString m_account_server;
    QString m_client_agent;
    QString m_server_version;

    std::vector<ManifestDirectory> m_directories;

    QEventLoop m_event_loop;
    int m_update_file_total;
    int m_update_file_number;
    QTime m_download_time;
    QNetworkReply *m_download_reply;
    QFile *m_download_output;

    void add_directory(ManifestDirectory directory);

    void parse_manifest(QByteArray data);
    ManifestDirectory parse_manifest_directory(QXmlStreamReader &reader);
    ManifestFile parse_manifest_file(QXmlStreamReader &reader);
};
