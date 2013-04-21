#pragma once

#include <QObject>
#include <QRunnable>

class Worker :  public QObject, public QRunnable
{
    Q_OBJECT

public:
    typedef unsigned char Data[16];

    Worker(const Data& data);
    ~Worker();

    virtual void run();

signals:
    void finished(const QVector<unsigned char>& result);

private:
    Data m_data;
};

