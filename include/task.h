#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QDateTime>

struct Task
{
    int id;
    QString name;
    QDateTime startTime;
    QString priority;
    QString category;
    QDateTime reminderTime;
    QString owner;

    Task()
        : id(0),
          priority("中"),
          category("生活")
    {
    }
};

#endif // TASK_H
