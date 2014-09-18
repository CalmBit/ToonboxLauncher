#include "DraggableWindow.h"

#include <QWidget>
#include <QMainWindow>
#include <QPoint>
#include <QMouseEvent>
#include <Qt>

DraggableWindow::DraggableWindow(QWidget *parent) : QMainWindow(parent),
    m_captured(false), m_last_mouse_pos(QPoint(0, 0))
{
}

void DraggableWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) {
        m_last_pos = event->pos();
        m_captured = true;
    }
}

void DraggableWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(m_captured) {
        this->move(this->pos() + (event->pos()-m_last_pos));
    }
}

void DraggableWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) {
        m_captured = false;
    }
}
