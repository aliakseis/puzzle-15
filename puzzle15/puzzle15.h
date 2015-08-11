#ifndef PUZZLE15_H
#define PUZZLE15_H

#include <QMainWindow>
#include "ui_puzzle15.h"

#include <QPropertyAnimation>

#include <memory>

class MovableButton;

class puzzle15 : public QMainWindow
{
    Q_OBJECT

public:
    puzzle15(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~puzzle15();

    void onButtonRelease(MovableButton* button, int offsetX, int offsetY);

 public slots:
    void onFinished(const QVector<unsigned char>& result);
    void animationStep();

protected:
    virtual void keyPressEvent(QKeyEvent* event);

private:
	void AnimatePiece(int row, int col);

private:
    Ui::puzzle15Class ui;
    int emptyX, emptyY;

    std::auto_ptr<QPropertyAnimation> animation;

    QVector<unsigned char> result;
    QVector<unsigned char>::iterator solution;
};

#endif // PUZZLE15_H
