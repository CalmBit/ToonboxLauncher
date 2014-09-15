#include "PatchDirectory.h"
#include "PatchFile.h"

#include <vector>

#include <QString>

PatchDirectory::PatchDirectory(QString name) : m_name(name)
{
}

void PatchDirectory::set_name(QString name)
{
    m_name = name;
}

QString PatchDirectory::get_name()
{
    return m_name;
}

void PatchDirectory::add_file(PatchFile file)
{
    m_files.push_back(file);
}

std::vector<PatchFile> PatchDirectory::get_files()
{
    return m_files;
}
