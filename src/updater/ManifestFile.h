#pragma once
#include <QString>
#include <QtGlobal>
#include <QByteArray>

class ManifestFile
{
  public:
    ManifestFile(const QString &name, qint64 size, const QByteArray &hash);

    void set_name(const QString &name);
    QString get_name();
    void set_size(qint64 size);
    qint64 get_size();
    void set_hash(const QByteArray &hash);
    QByteArray get_hash();

    void set_path(const QString &path);
    QString get_path();

  private:
    QString m_name;
    qint64 m_size;
    QByteArray m_hash;

    QString m_path;
};
