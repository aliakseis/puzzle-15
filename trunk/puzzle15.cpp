#include "puzzle15.h"

#include "MovableButton.h"

#include "Worker.h"

#include <QKeyEvent>
#include <QThreadPool>


const char idleStatus[] = "Hit F11 to start computation ...";

const char animatingStatus[] = "Animating ...";

const char computingStatus[] = "Computing ...";

puzzle15::puzzle15(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
    , emptyX(3), emptyY(3)
{
    qRegisterMetaType< QVector<unsigned char> >("QVector<unsigned char>");

    ui.setupUi(this);

    QHeaderView* horizontalHeader = ui.centralWidget->horizontalHeader(); 
    horizontalHeader->setResizeMode(QHeaderView::Stretch); 

    QHeaderView* verticalHeader = ui.centralWidget->verticalHeader(); 
    verticalHeader->setResizeMode(QHeaderView::Stretch); 

	// Now add the buttons in
	int i = 1;
	for (int row = 0; row < 4; row++)
	{
		for (int col = 0; col < 4; col++)
		{
			// lower right cell is empty
			if (row == 4 - 1 && col == 4 - 1)
			{
				continue;
			}

            MovableButton* button = new MovableButton(this, row, col);

            QString text;
            text.sprintf("%d", i);
            button->setText(text);

            ui.centralWidget->setCellWidget(row, col, button);

            i++;
		}
	}
    MovableButton* button = new MovableButton(this, 3, 3);
    ui.centralWidget->setCellWidget(3, 3, button);
    button->setFlat(true);
    button->setEnabled(false);

    statusBar()->showMessage(idleStatus);
}

puzzle15::~puzzle15()
{

}


void puzzle15::onButtonRelease(MovableButton* buttonBeingMoved, int offsetX, int offsetY)
{
    if (!result.empty())
    {
        statusBar()->showMessage(idleStatus);
        result.clear(); // Stop animation
    }

    int row = buttonBeingMoved->row();
    int col = buttonBeingMoved->col();

    int newCol, newRow;
    if (offsetX != 0 || offsetY != 0)
    {
        newCol = col + offsetX;
        newRow = row + offsetY;

        if (newCol < 0)
            newCol = 0;
        else if (newCol > 3)
            newCol = 3;

        if (newRow < 0)
            newRow = 0;
        else if (newRow > 3)
            newRow = 3;

		if (emptyX == newCol && emptyY == newRow)
		{

			if (offsetX != 0 || offsetY != 0)
			{
				emptyX = col;
				emptyY = row;

                QWidget* oldBtn = ui.centralWidget->cellWidget(row, col);
                QWidget* newBtn = ui.centralWidget->cellWidget(newRow, newCol);
                QVariant propText = oldBtn->property("text");
                newBtn->setProperty("text", propText);
                propText = "";
                oldBtn->setProperty("text", propText);

                oldBtn->setProperty("flat", true);
                oldBtn->setEnabled(false);

                newBtn->setProperty("flat", false);
                newBtn->setEnabled(true);
			}
		}
		else
		{
            QWidget* oldBtn = ui.centralWidget->cellWidget(row, col);
            QWidget* newBtn = ui.centralWidget->cellWidget(newRow, newCol);

            QVariant propOldText = oldBtn->property("text");
            QVariant propNewText = newBtn->property("text");
            newBtn->setProperty("text", propOldText);
            oldBtn->setProperty("text", propNewText);
		}
    }
    else
    {
        newCol = emptyX;
        newRow = emptyY;

        if (newCol == col)
        {
            if (newRow == row)
                return;
            if (newRow == row + 1)
            {
                emptyX = col;
                emptyY = row;
				AnimatePiece(newRow, newCol, Down);
                return;
            }
            if (newRow == row - 1)
            {
                emptyX = col;
                emptyY = row;
				AnimatePiece(newRow, newCol, Up);
                return;
            }
        }

        if (newRow == row)
        {
            if (newCol == col + 1)
            {
                emptyX = col;
                emptyY = row;
				AnimatePiece(newRow, newCol, Right);
                return;
            }
            if (newCol == col - 1)
            {
                emptyX = col;
                emptyY = row;
				AnimatePiece(newRow, newCol, Left);
                return;
            }
        }
    }
}

void puzzle15::AnimatePiece(int row, int col, MoveStatus moveStatus)
{
    QWidget* oldBtn = ui.centralWidget->cellWidget(emptyY, emptyX);
    QWidget* newBtn = ui.centralWidget->cellWidget(row, col);
    QVariant propText = oldBtn->property("text");
    newBtn->setProperty("text", propText);
    propText = "";
    oldBtn->setProperty("text", propText);

    oldBtn->setProperty("flat", true);
    oldBtn->setEnabled(false);

    newBtn->setProperty("flat", false);
    newBtn->setEnabled(true);

    animation.reset(new QPropertyAnimation(newBtn, "geometry"));

    animation->setDuration(300);
    QRect oldRect(oldBtn->rect());
    oldRect.moveTo(oldBtn->pos());
    QRect newRect(newBtn->rect());
    newRect.moveTo(newBtn->pos());
    animation->setStartValue(oldRect);
    animation->setEndValue(newRect);

    connect(animation.get(), SIGNAL(finished()), SLOT(animationStep()));

    animation->start();
}

void puzzle15::keyPressEvent(QKeyEvent* event)
{
    QMainWindow::keyPressEvent(event);
    if (122 != event->nativeVirtualKey()) // F11
    {
        return;
    }

    unsigned char data[16] = { 0 };

	for (int row = 0; row < 4; row++)
	{
		for (int col = 0; col < 4; col++)
        {
            QWidget* btn = ui.centralWidget->cellWidget(row, col);
            QVariant propText = btn->property("text");

            bool ok = false;
            int cell = propText.toInt(&ok);
            if (ok)
            {
        		data[row * 4 + col] = cell;
            }
        }
    }

    Worker* worker = new Worker(data);

    connect(worker, SIGNAL(finished(QVector<unsigned char>)), SLOT(onFinished(QVector<unsigned char>)));

    QThreadPool::globalInstance()->start(worker);

    statusBar()->showMessage(computingStatus);

    QApplication::setOverrideCursor(Qt::WaitCursor);
    setEnabled(false);
}

void puzzle15::onFinished(const QVector<unsigned char>& res)
{
    setEnabled(true);
    QApplication::restoreOverrideCursor();    

    result = res;
    if (!result.empty())
    {
        statusBar()->showMessage(animatingStatus);
        solution = result.begin();
        animationStep();
    }
    else
    {
        statusBar()->showMessage(idleStatus);
    }
}

void puzzle15::animationStep()
{
	if (result.empty())
		return;

    if (result.end() == solution)
    {
        statusBar()->showMessage(idleStatus);
        result.clear();
        return;
    }

    MoveStatus nextMove = (MoveStatus) *solution;
    ++solution;

	int newCol = emptyX;
    int newRow = emptyY;

	switch (nextMove)
	{
	case Left: emptyX++; break;
	case Right: emptyX--; break;
	case Up: emptyY++; break;
	case Down: emptyY--; break;
	}

	AnimatePiece(newRow, newCol, nextMove);
}
