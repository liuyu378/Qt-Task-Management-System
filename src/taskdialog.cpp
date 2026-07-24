#include "taskdialog.h"

#include <QLineEdit>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>

TaskDialog::TaskDialog(QWidget* parent)
    : QDialog(parent),
      taskId_(0)
{
    setWindowTitle("任务信息");
    resize(400, 260);

    nameEdit_ = new QLineEdit(this);

    startTimeEdit_ = new QDateTimeEdit(this);
    startTimeEdit_->setCalendarPopup(true);
    startTimeEdit_->setDisplayFormat("yyyy-MM-dd HH:mm");
    startTimeEdit_->setDateTime(QDateTime::currentDateTime());

    priorityBox_ = new QComboBox(this);
    priorityBox_->addItem("低");
    priorityBox_->addItem("中");
    priorityBox_->addItem("高");

    categoryBox_ = new QComboBox(this);
    categoryBox_->addItem("生活");
    categoryBox_->addItem("学习");
    categoryBox_->addItem("工作");
    categoryBox_->addItem("其他");

    reminderTimeEdit_ = new QDateTimeEdit(this);
    reminderTimeEdit_->setCalendarPopup(true);
    reminderTimeEdit_->setDisplayFormat("yyyy-MM-dd HH:mm");
    reminderTimeEdit_->setDateTime(QDateTime::currentDateTime());

    QPushButton* okButton = new QPushButton("确定", this);
    QPushButton* cancelButton = new QPushButton("取消", this);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow("任务名称：", nameEdit_);
    formLayout->addRow("开始时间：", startTimeEdit_);
    formLayout->addRow("优先级：", priorityBox_);
    formLayout->addRow("分类：", categoryBox_);
    formLayout->addRow("提醒时间：", reminderTimeEdit_);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void TaskDialog::setTask(const Task& task)
{
    taskId_ = task.id;
    owner_ = task.owner;

    nameEdit_->setText(task.name);
    startTimeEdit_->setDateTime(task.startTime);
    priorityBox_->setCurrentText(task.priority);
    categoryBox_->setCurrentText(task.category);
    reminderTimeEdit_->setDateTime(task.reminderTime);
}

Task TaskDialog::getTask() const
{
    Task task;

    task.id = taskId_;
    task.owner = owner_;
    task.name = nameEdit_->text();
    task.startTime = startTimeEdit_->dateTime();
    task.priority = priorityBox_->currentText();
    task.category = categoryBox_->currentText();
    task.reminderTime = reminderTimeEdit_->dateTime();

    return task;
}
