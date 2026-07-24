#ifndef TASKDIALOG_H
#define TASKDIALOG_H

#include <QDialog>
#include "task.h"

class QLineEdit;
class QDateTimeEdit;
class QComboBox;

class TaskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TaskDialog(QWidget* parent = nullptr);

    void setTask(const Task& task);
    Task getTask() const;

private:
    QLineEdit* nameEdit_;
    QDateTimeEdit* startTimeEdit_;
    QComboBox* priorityBox_;
    QComboBox* categoryBox_;
    QDateTimeEdit* reminderTimeEdit_;

    int taskId_;
    QString owner_;
};

#endif // TASKDIALOG_H
