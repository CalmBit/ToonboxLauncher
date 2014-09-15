#pragma once
#include "PatchFile.h"

#include <vector>

#include <QString>

class PatchDirectory
{
    public:
        PatchDirectory(QString name);

        void set_name(QString name);
        QString get_name();

        void add_file(PatchFile file);
        std::vector<PatchFile> get_files();

    private:
        QString m_name;
        std::vector<PatchFile> m_files;
};
