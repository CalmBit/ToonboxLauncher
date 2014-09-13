#include "Patcher.h"

#include <QString>
#include <QUrl>

Patcher::Patcher(QString url) : m_url(url)
{
}

Patcher::Patcher(QUrl url) : m_url(url)
{
}

void Patcher::set_url(QString url)
{
    m_url = QUrl(url);
}

void Patcher::set_url(QUrl url)
{
    m_url = url;
}

QUrl Patcher::get_url()
{
    return m_url;
}
