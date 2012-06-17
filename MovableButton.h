#pragma once

#include <qpushbutton.h>

class puzzle15;

class MovableButton :
    public QPushButton
{
    Q_OBJECT
    //Q_PROPERTY(int row READ row WRITE setRow)
    //Q_PROPERTY(int col READ col WRITE setCol) 

public:
    MovableButton(puzzle15* parent, int row, int col);
    ~MovableButton();
    
    int row() { return m_row; }
    //void setRow(int row) { m_row = row; }
    int col() { return m_col; }
    //void setCol(int col) { m_col = col; }
    


protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    puzzle15* m_parent;
    int m_row;
    int m_col;

    QPoint start;
    QPoint moveStartPos;
    bool moving;
};
