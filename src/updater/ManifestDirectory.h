#pragma once
#include "ManifestFile.h"

#include <vector>

#include <QString>

class ManifestDirectory
{
  public:
    ManifestDirectory(QString name);

    void set_name(QString name);
    QString get_name();

    void add_directory(ManifestDirectory directory);
    std::vector<ManifestDirectory> get_directories();
    void add_file(ManifestFile file);
    std::vector<ManifestFile> get_files();

    void set_path(QString path);
    QString get_path();

  private:
    QString m_name;

    std::vector<ManifestDirectory> m_directories;
    std::vector<ManifestFile> m_files;

    QString m_path;
};
