#pragma once
#include "PatchFile.h"

#include <vector>

class PatchDirectory
{
    public:
        PatchDirectory(QString path);

        void set_path(QString path);
        QString get_path();

        void add_file(PatchFile file);
        std::vector<PatchFile> get_files();

    private:
        QString m_path;
        std::vector<PatchFile> m_files;
};
