#include "MovableButton.h"

#include "puzzle15.h"

#include <QMouseEvent>

MovableButton::MovableButton(puzzle15* parent, int row, int col)
    : m_parent(parent)
    , m_row(row)
    , m_col(col)
    , moving(false)
{
}


MovableButton::~MovableButton()
{
}

void MovableButton::mousePressEvent(QMouseEvent *event)
{
    QPushButton::mousePressEvent(event);
    
    if((event->button() == Qt::LeftButton)) {
        moving = true;
        start = this->pos();
        moveStartPos = event->globalPos();
    }
}

void MovableButton::mouseMoveEvent(QMouseEvent *event)
{
    QPushButton::mouseMoveEvent(event);

    if(moving)
    {
        raise();
        move(start + event->globalPos() - moveStartPos);
    }
}

void MovableButton::mouseReleaseEvent(QMouseEvent *event)
{
    QPushButton::mouseReleaseEvent(event);

    if(event->button() == Qt::LeftButton) {
        moving = false;

        QPoint movePos = event->globalPos();

        const int pieceColWidth(size().width());
        const int pieceRowHeight(size().height());

        int offsetX;
        if (movePos.x() > moveStartPos.x())
            offsetX = (int)((movePos.x() - moveStartPos.x() + pieceColWidth / 2) / pieceColWidth);
        else
            offsetX = (int)((movePos.x() - moveStartPos.x() - pieceColWidth / 2) / pieceColWidth);

        int offsetY;
        if (movePos.y() > moveStartPos.y())
            offsetY = (int)((movePos.y() - moveStartPos.y() + pieceRowHeight / 2) / pieceRowHeight);
        else
            offsetY = (int)((movePos.y() - moveStartPos.y() - pieceRowHeight / 2) / pieceRowHeight);

        move(start);

        m_parent->onButtonRelease(this, offsetX, offsetY);
    }
}
