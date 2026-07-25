#include "taskdialog.h"

#include <QLineEdit>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QProcess>
#include <QMessageBox>
#include <QRegularExpression>

// 构造函数
// 创建任务名称、开始时间、优先级、分类、提醒时间等
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
    priorityBox_->setCurrentText("中");

    categoryBox_ = new QComboBox(this);
    categoryBox_->addItem("生活");
    categoryBox_->addItem("学习");
    categoryBox_->addItem("工作");
    categoryBox_->addItem("其他");
    categoryBox_->setCurrentText("生活");

    reminderTimeEdit_ = new QDateTimeEdit(this);
    reminderTimeEdit_->setCalendarPopup(true);
    reminderTimeEdit_->setDisplayFormat("yyyy-MM-dd HH:mm");
    reminderTimeEdit_->setDateTime(QDateTime::currentDateTime());
    
    voiceButton_ = new QPushButton("语音录入",this);
    QPushButton* okButton = new QPushButton("确定", this);
    QPushButton* cancelButton = new QPushButton("取消", this);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow("任务名称：", nameEdit_);
    formLayout->addRow("开始时间：", startTimeEdit_);
    formLayout->addRow("优先级：", priorityBox_);
    formLayout->addRow("分类：", categoryBox_);
    formLayout->addRow("提醒时间：", reminderTimeEdit_);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(voiceButton_);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);
    
    connect(voiceButton_,&QPushButton::clicked,this,&TaskDialog::onVoiceInput);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

// 将已有任务信息填入对话框
// 用于编辑任务时显示原任务内容
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

// 从对话框控件中读取用户输入，组装成 Task 对象
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

// 语音识别函数
void TaskDialog::onVoiceInput()
{
    voiceButton_->setEnabled(false);
    voiceButton_->setText("识别中...");

    QProcess process;

    QString program = "python3";
    QStringList arguments;
    arguments << "../scripts/voice_recognize.py";

    process.start(program, arguments);

    if (!process.waitForStarted(3000)) {
        QMessageBox::warning(this, "语音识别失败", "无法启动语音识别脚本。");
        voiceButton_->setEnabled(true);
        voiceButton_->setText("语音录入");
        return;
    }

    // 最多等待 10 秒
    if (!process.waitForFinished(10000)) {
        process.kill();
        QMessageBox::warning(this, "语音识别超时", "没有识别到有效语音。");
        voiceButton_->setEnabled(true);
        voiceButton_->setText("语音录入");
        return;
    }

    QString text = QString::fromUtf8(process.readAllStandardOutput()).trimmed();

    voiceButton_->setEnabled(true);
    voiceButton_->setText("语音录入");

    if (text.isEmpty()) {
        QMessageBox::warning(this, "语音识别失败", "没有识别到有效内容。");
        return;
    }

    QMessageBox::information(this, "识别结果", "识别到：\n" + text);

    applyVoiceText(text);
}

void TaskDialog::applyVoiceText(const QString& text)
{
    QString taskName = text;

    QDateTime dateTime = QDateTime::currentDateTime();

    // 解析日期：今天 / 明天
    if (text.contains("明天")) {
        dateTime.setDate(QDate::currentDate().addDays(1));
    } else if (text.contains("今天")) {
        dateTime.setDate(QDate::currentDate());
    }

    // 解析小时
    int hour = -1;

    if (text.contains("一点")) hour = 1;
    else if (text.contains("两点") || text.contains("二点")) hour = 2;
    else if (text.contains("三点")) hour = 3;
    else if (text.contains("四点")) hour = 4;
    else if (text.contains("五点")) hour = 5;
    else if (text.contains("六点")) hour = 6;
    else if (text.contains("七点")) hour = 7;
    else if (text.contains("八点")) hour = 8;
    else if (text.contains("九点")) hour = 9;
    else if (text.contains("十点")) hour = 10;
    else if (text.contains("十一点")) hour = 11;
    else if (text.contains("十二点")) hour = 12;

    // 下午 / 晚上自动加 12 小时
    if (hour > 0 && (text.contains("下午") || text.contains("晚上"))) {
        if (hour < 12) {
            hour += 12;
        }
    }

    if (hour > 0) {
        QTime time(hour, 0);
        dateTime.setTime(time);

        startTimeEdit_->setDateTime(dateTime);
        reminderTimeEdit_->setDateTime(dateTime);
    }

    // 简单提取任务名称
    // 例如：“明天下午三点提醒我写作业”
    taskName.replace("今天", "");
    taskName.replace("明天", "");
    taskName.replace("上午", "");
    taskName.replace("下午", "");
    taskName.replace("晚上", "");
    taskName.replace("提醒我", "");
    taskName.replace("提醒", "");

    taskName.replace("一点", "");
    taskName.replace("两点", "");
    taskName.replace("二点", "");
    taskName.replace("三点", "");
    taskName.replace("四点", "");
    taskName.replace("五点", "");
    taskName.replace("六点", "");
    taskName.replace("七点", "");
    taskName.replace("八点", "");
    taskName.replace("九点", "");
    taskName.replace("十点", "");
    taskName.replace("十一点", "");
    taskName.replace("十二点", "");

    taskName = taskName.trimmed();

    if (!taskName.isEmpty()) {
        nameEdit_->setText(taskName);
    }

    // 根据关键词设置分类
    if (text.contains("学习") || text.contains("作业") || text.contains("复习")) {
        categoryBox_->setCurrentText("学习");
    } else if (text.contains("工作") || text.contains("会议")) {
        categoryBox_->setCurrentText("工作");
    } else {
        categoryBox_->setCurrentText("生活");
    }

    // 根据关键词设置优先级
    if (text.contains("重要") || text.contains("紧急")) {
        priorityBox_->setCurrentText("高");
    } else {
        priorityBox_->setCurrentText("中");
    }
}
