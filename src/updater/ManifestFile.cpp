#include "ManifestFile.h"

#include <QString>
#include <QtGlobal>
#include <QByteArray>

ManifestFile::ManifestFile(QString name, qint64 size, QByteArray hash) :
    m_name(name), m_size(size), m_hash(hash), m_path(name)
{
}

void ManifestFile::set_name(QString name)
{
    m_name = name;
}

QString ManifestFile::get_name()
{
    return m_name;
}

void ManifestFile::set_size(qint64 size)
{
    m_size = size;
}

qint64 ManifestFile::get_size()
{
    return m_size;
}

void ManifestFile::set_hash(QByteArray hash)
{
    m_hash = hash;
}

QByteArray ManifestFile::get_hash()
{
    return m_hash;
}

void ManifestFile::set_path(QString path)
{
    m_path = path;
}

QString ManifestFile::get_path()
{
    return m_path;
}
