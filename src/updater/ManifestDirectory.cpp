#include "ManifestDirectory.h"
#include "ManifestFile.h"

#include <vector>

#include <QString>

ManifestDirectory::ManifestDirectory(QString name) : m_name(name), m_path(name)
{
}

void ManifestDirectory::set_name(QString name)
{
    m_name = name;
}

QString ManifestDirectory::get_name()
{
    return m_name;
}

void ManifestDirectory::add_directory(ManifestDirectory directory)
{
    directory.set_path(m_path + "/" + directory.get_name());
    m_directories.push_back(directory);
}

std::vector<ManifestDirectory> ManifestDirectory::get_directories()
{
    return m_directories;
}

void ManifestDirectory::add_file(ManifestFile file)
{
    file.set_path(m_path + "/" + file.get_name());
    m_files.push_back(file);
}

std::vector<ManifestFile> ManifestDirectory::get_files()
{
    return m_files;
}

void ManifestDirectory::set_path(QString path)
{
    m_path = path;
}

QString ManifestDirectory::get_path()
{
    return m_path;
}
