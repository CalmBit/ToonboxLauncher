#pragma once
#include <QMainWindow>
#include <QWidget>
#include <QMouseEvent>
#include <QPoint>

class DraggableWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit DraggableWindow(QWidget *parent = 0);

  protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

  private:
    bool m_captured;
    QPoint m_last_event_pos;
};
