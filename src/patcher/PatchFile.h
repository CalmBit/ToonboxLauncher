#pragma once
#include <QString>

class PatchFile
{
    public:
        PatchFile(QString name, ulong size, QString hash);

        bool operator==(PatchFile &other);
        bool operator!=(PatchFile &other);

        void set_name(QString name);
        QString get_name();

        void set_size(ulong size);
        ulong get_size();

        void set_hash(QString hash);
        QString get_hash();

    private:
        QString m_name;
        ulong m_size;
        QString m_hash;
};
