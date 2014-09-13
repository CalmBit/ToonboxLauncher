#pragma once
#include <QString>
#include <QUrl>

class Patcher
{
    public:
        Patcher(QString url);
        Patcher(QUrl url);

        void set_url(QString url);
        void set_url(QUrl url);
        QUrl get_url();

    private:
        QUrl m_url;
};
