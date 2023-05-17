#include "Worker.h"

#include "../solver/solver.h"

#include <QVector>
#include <memory.h>

Worker::Worker(const Data& data)
{
    memcpy(m_data, data, sizeof(Data));
}


Worker::~Worker()
= default;

void Worker::run()
{
    QVector<unsigned char> result(100);

    int solved = Solve(m_data, &result[0]);

    if (solved < 0)
    {
        solved = 0;
    }

    result.erase(result.begin() + solved, result.end());

    emit finished(result);
}
