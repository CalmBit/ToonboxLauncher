#pragma once
#include <stdint.h>

#include <QString>

class PatchFile
{
    public:
        PatchFile(QString name, uintmax_t size, QString hash);

        bool operator==(PatchFile &other);
        bool operator!=(PatchFile &other);

        void set_name(QString name);
        QString get_name();

        void set_size(uintmax_t size);
        uintmax_t get_size();

        void set_hash(QString hash);
        QString get_hash();

    private:
        QString m_name;
        uintmax_t m_size;
        QString m_hash;
};
