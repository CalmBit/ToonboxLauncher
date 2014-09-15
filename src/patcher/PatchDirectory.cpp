#include "PatchDirectory.h"
#include "PatchFile.h"

#include <vector>

#include <QString>

PatchDirectory::PatchDirectory(QString path) : m_path(path)
{
}

void PatchDirectory::set_path(QString path)
{
    m_path = path;
}

QString PatchDirectory::get_path()
{
    return m_path;
}

void PatchDirectory::add_file(PatchFile file)
{
    m_files.push_back(file);
}

std::vector<PatchFile> PatchDirectory::get_files()
{
    return m_files;
}
