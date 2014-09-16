#include "PatchFile.h"

#include <QString>

PatchFile::PatchFile(QString name, ulong size, QString hash) :
    m_name(name), m_size(size), m_hash(hash)
{
}

bool PatchFile::operator==(PatchFile &other)
{
    return (this->get_size() == other.get_size() &&
            this->get_hash() == other.get_hash());
}

bool PatchFile::operator!=(PatchFile &other)
{
    return (this->get_size() != other.get_size() &&
            this->get_hash() != other.get_hash());
}

void PatchFile::set_name(QString name)
{
    m_name = name;
}

QString PatchFile::get_name()
{
    return m_name;
}

void PatchFile::set_size(ulong size)
{
    m_size = size;
}

ulong PatchFile::get_size()
{
    return m_size;
}

void PatchFile::set_hash(QString hash)
{
    m_hash = hash;
}

QString PatchFile::get_hash()
{
    return m_hash;
}
