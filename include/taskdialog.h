#ifndef TASKDIALOG_H
#define TASKDIALOG_H

#include <QDialog>
#include "task.h"

class QLineEdit;
class QDateTimeEdit;
class QComboBox;
class QPushButton;
class QCheckBox;
class TaskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TaskDialog(QWidget* parent = nullptr);

    void setTask(const Task& task);
    Task getTask() const;
private slots:
	void onVoiceInput();   // 点击"语音录入"按钮后调用
	void onAcceptClicked();    // 点击确定按钮后，先检查输入是否合法
private:
    void applyVoiceText(const QString& text);

    QLineEdit* nameEdit_;
    QDateTimeEdit* startTimeEdit_;
    QComboBox* priorityBox_;
    QComboBox* categoryBox_;
    QCheckBox* reminderCheckBox_;   //是否启用提醒
    QDateTimeEdit* reminderTimeEdit_;
    
    QPushButton* voiceButton_;   //语音录入按钮
    int taskId_;
    QString owner_;
};

#endif // TASKDIALOG_H
