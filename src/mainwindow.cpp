#include "mainwindow.h"
#include "taskdialog.h"

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QDate>
#include <QDateTime>
#include <QMetaObject>
#include <QDebug>

#include <chrono>
#include <thread>
#include <QSoundEffect>
#include <QUrl>
#include <QFileInfo>
#include <QApplication>

MainWindow::MainWindow(const QString& username, QWidget* parent)
    : QMainWindow(parent),
      username_(username),
      table_(nullptr),
      addButton_(nullptr),
      editButton_(nullptr),
      deleteButton_(nullptr),
      refreshButton_(nullptr),
      logoutButton_(nullptr),
      aboutButton_(nullptr),
      filterBox_(nullptr),
      userLabel_(nullptr),
      reminderSound_(nullptr),
      running_(false)
{
    // 先从本地文件加载任务到 taskMgr_ 内存中
    // 这一步就是“用户登录后，从文件加载任务列表，保存到内存”
    storage_.init();
    storage_.loadTasks(username_, taskMgr_);

    // 初始化主界面
    setupUiExtra();

    // 初始化提醒音效
    reminderSound_ = new QSoundEffect(this);

    // 提醒音频文件路径
    // 程序在 build 目录运行，所以 ../resources/reminder.wav 指向项目根目录下的 resources/reminder.wav
    QString soundPath = "../resources/reminder_fixed.wav";

    if (QFile::exists(soundPath)) {
    reminderSound_->setSource(QUrl::fromLocalFile(soundPath));
    reminderSound_->setLoopCount(1);
    reminderSound_->setVolume(1.0);
    qDebug() << "提醒音效已设置:" << reminderSound_->source();
    }
    else{qDebug() << "提醒音频文件不存在";}
    // 把内存中的任务显示到表格
    loadTasks();
    
    // 启动后台提醒线程
    startReminderThread();

    // 程序进入主界面后立即检查一次提醒
    checkReminders();
}

MainWindow::~MainWindow()
{
    // 停止后台提醒线程
    stopReminderThread();
}

void MainWindow::setupUiExtra()
{
    setWindowTitle("MySchedule - " + username_);
    resize(900, 600);

    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    userLabel_ = new QLabel("当前用户：" + username_, this);

    filterBox_ = new QComboBox(this);
    filterBox_->addItem("全部任务");
    filterBox_->addItem("今日任务");
    filterBox_->addItem("本月任务");

    addButton_ = new QPushButton("添加任务", this);
    editButton_ = new QPushButton("编辑任务", this);
    deleteButton_ = new QPushButton("删除任务", this);
    refreshButton_ = new QPushButton("刷新", this);
    aboutButton_ = new QPushButton("关于", this);
    logoutButton_ = new QPushButton("退出", this);

    table_ = new QTableWidget(this);
    table_->setColumnCount(6);
    table_->setHorizontalHeaderLabels(
        {"ID", "任务名称", "开始时间", "优先级", "分类", "提醒时间"}
    );

    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setAlternatingRowColors(true);
    table_->verticalHeader()->setVisible(false);

    QHBoxLayout* topLayout = new QHBoxLayout;
    topLayout->addWidget(userLabel_);
    topLayout->addStretch();
    topLayout->addWidget(new QLabel("筛选：", this));
    topLayout->addWidget(filterBox_);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(addButton_);
    buttonLayout->addWidget(editButton_);
    buttonLayout->addWidget(deleteButton_);
    buttonLayout->addWidget(refreshButton_);
    buttonLayout->addStretch();
    buttonLayout->addWidget(aboutButton_);
    buttonLayout->addWidget(logoutButton_);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(table_);
    mainLayout->addLayout(buttonLayout);

    connect(addButton_, &QPushButton::clicked, this, &MainWindow::onAddTask);
    connect(editButton_, &QPushButton::clicked, this, &MainWindow::onEditTask);
    connect(deleteButton_, &QPushButton::clicked, this, &MainWindow::onDeleteTask);
    connect(refreshButton_, &QPushButton::clicked, this, &MainWindow::onRefresh);
    connect(logoutButton_, &QPushButton::clicked, this, &MainWindow::onLogout);
    connect(aboutButton_, &QPushButton::clicked, this, &MainWindow::onAbout);

    connect(
        filterBox_,
        &QComboBox::currentIndexChanged,
        this,
        &MainWindow::onFilterChanged
    );
}

void MainWindow::loadTasks()
{
    QVector<Task> allTasks = taskMgr_.getTasksByUser(username_);
    QVector<Task> result;

    int filterIndex = filterBox_ ? filterBox_->currentIndex() : 0;
    QDate today = QDate::currentDate();

    for (const Task& task : allTasks) {
        if (filterIndex == 0) {
            result.append(task);
        } else if (filterIndex == 1) {
            if (task.startTime.date() == today) {
                result.append(task);
            }
        } else if (filterIndex == 2) {
            QDate d = task.startTime.date();
            if (d.year() == today.year() && d.month() == today.month()) {
                result.append(task);
            }
        }
    }

    refreshTable(result);
}

void MainWindow::refreshTable(const QVector<Task>& tasks)
{
    table_->setRowCount(tasks.size());

    for (int row = 0; row < tasks.size(); ++row) {
        const Task& t = tasks[row];

        table_->setItem(row, 0, new QTableWidgetItem(QString::number(t.id)));
        table_->setItem(row, 1, new QTableWidgetItem(t.name));
        table_->setItem(row, 2, new QTableWidgetItem(t.startTime.toString("yyyy-MM-dd HH:mm")));
        table_->setItem(row, 3, new QTableWidgetItem(t.priority));
        table_->setItem(row, 4, new QTableWidgetItem(t.category));

        QString reminderText;
        if (t.reminderTime.isValid()) {
            reminderText = t.reminderTime.toString("yyyy-MM-dd HH:mm");
        } else {
            reminderText = "无";
        }

        table_->setItem(row, 5, new QTableWidgetItem(reminderText));

        if (t.priority == "高") {
            for (int col = 0; col < table_->columnCount(); ++col) {
                table_->item(row, col)->setBackground(Qt::yellow);
            }
        }
    }
}

int MainWindow::currentSelectedTaskId() const
{
    int row = table_->currentRow();

    if (row < 0) {
        return -1;
    }

    QTableWidgetItem* item = table_->item(row, 0);

    if (!item) {
        return -1;
    }

    return item->text().toInt();
}

void MainWindow::onAddTask()
{
    TaskDialog dlg(this);

    if (dlg.exec() == QDialog::Accepted) {
        Task task = dlg.getTask();

        task.owner = username_;

        QString errorMessage;
        if (!validateTaskForUi(task, -1, errorMessage)) {
            QMessageBox::warning(this, "添加失败", errorMessage);
            return;
        }

        if (taskMgr_.addTask(task)) {
            storage_.saveTasks(username_, taskMgr_);
            loadTasks();
            QMessageBox::information(this, "成功", "任务添加成功！");
        } else {
            QMessageBox::warning(this, "添加失败", "任务添加失败，请检查任务信息是否重复。");
        }
    }
}

void MainWindow::onEditTask()
{
    int taskId = currentSelectedTaskId();

    if (taskId <= 0) {
        QMessageBox::information(this, "提示", "请先选择一个任务。");
        return;
    }

    QVector<Task> tasks = taskMgr_.getTasksByUser(username_);

    Task oldTask;
    bool found = false;

    for (const Task& task : tasks) {
        if (task.id == taskId) {
            oldTask = task;
            found = true;
            break;
        }
    }

    if (!found) {
        QMessageBox::warning(this, "错误", "没有找到该任务。");
        return;
    }

    TaskDialog dlg(this);
    dlg.setTask(oldTask);

    if (dlg.exec() == QDialog::Accepted) {
        Task newTask = dlg.getTask();

	newTask.id = oldTask.id;
	newTask.owner = username_;

	QString errorMessage;
	if (!validateTaskForUi(newTask, oldTask.id, errorMessage)) {
	    QMessageBox::warning(this, "修改失败", errorMessage);
	    return;
	}

	taskMgr_.deleteTask(oldTask.id);

	if (!taskMgr_.addTask(newTask)) {
	    taskMgr_.addTask(oldTask);
	    QMessageBox::warning(this, "修改失败", "任务修改失败，请检查任务信息。");
	    return;
	}
	
	remindedTaskIds_.remove(oldTask.id);    // 若一个任务已经提醒过，编辑了新的提醒时间，后续还能再次提醒

	// 每次编辑完成后，立即保存到文件
	storage_.saveTasks(username_, taskMgr_);
	
	loadTasks();
	}
    }


void MainWindow::onDeleteTask()
{
    int taskId = currentSelectedTaskId();

    if (taskId <= 0) {
        QMessageBox::information(this, "提示", "请先选择一个任务。");
        return;
    }

    int ret = QMessageBox::question(this, "确认删除", "确定要删除这个任务吗？");

    if (ret == QMessageBox::Yes) {
        if (taskMgr_.deleteTask(taskId)) {

	remindedTaskIds_.remove(taskId);
	
            // 每次删除完成后，立即保存到文件
            storage_.saveTasks(username_, taskMgr_);

            loadTasks();
        } else {
            QMessageBox::warning(this, "删除失败", "没有找到该任务。");
        }
    }
}

void MainWindow::onRefresh()
{
    loadTasks();
}

void MainWindow::onFilterChanged()
{
    loadTasks();
}

void MainWindow::checkReminders()
{
    QVector<Task> tasks = taskMgr_.getTasksByUser(username_);
    QDateTime now = QDateTime::currentDateTime();

    for (const Task& task : tasks) {
        // 没有设置提醒时间，不提醒
        if (!task.reminderTime.isValid()) {
            continue;
        }

        // 已经提醒过的任务不重复提醒
        if (remindedTaskIds_.contains(task.id)) {
            continue;
        }

        // 到达提醒时间
        if (task.reminderTime <= now) {
            remindedTaskIds_.insert(task.id);

            // 控制台打印提醒，满足“屏幕打印提醒”
            qDebug() << "任务提醒："
                     << "任务名称:" << task.name
                     << "开始时间:" << task.startTime.toString("yyyy-MM-dd HH:mm")
                     << "提醒时间:" << task.reminderTime.toString("yyyy-MM-dd HH:mm")
                     << "优先级:" << task.priority
                     << "分类:" << task.category;

	    playReminderSound();

            // 弹窗提醒
            QMessageBox::information(
                this,
                "任务提醒",
                QString("任务：%1\n\n开始时间：%2\n提醒时间：%3\n优先级：%4\n分类：%5")
                    .arg(task.name)
                    .arg(task.startTime.toString("yyyy-MM-dd HH:mm"))
                    .arg(task.reminderTime.toString("yyyy-MM-dd HH:mm"))
                    .arg(task.priority)
                    .arg(task.category)
            );
        }
    }
}

void MainWindow::onLogout()
{
    storage_.saveTasks(username_, taskMgr_);
    close();
}

void MainWindow::onAbout()
{
    QMessageBox::about(
        this,
        "关于 MySchedule",
        "MySchedule 日程管理软件\n\n"
        "功能：\n"
        "- 用户注册和登录\n"
        "- 本地文件保存任务\n"
        "- 添加、编辑、删除任务\n"
        "- 今日任务 / 本月任务筛选\n"
        "- 任务提醒"
    );
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    storage_.saveTasks(username_, taskMgr_);
    stopReminderThread();
    event->accept();
}

void MainWindow::startReminderThread()
{
    // 防止重复启动线程
    if (running_) {
        return;
    }

    running_ = true;

    reminderThread_ = std::thread([this]() {
        while (running_) {
            // 每 30 秒检查一次
            // 拆成 30 次 1 秒，是为了关闭窗口时线程能更快退出
            for (int i = 0; i < 30 && running_; ++i) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            if (!running_) {
                break;
            }

            // 注意：
            // 后台线程不能直接操作 Qt 界面。
            // QMessageBox 必须在主线程中弹出。
            // 所以这里把 checkReminders 投递回 Qt 主线程执行。
            QMetaObject::invokeMethod(
                this,
                "checkReminders",
                Qt::QueuedConnection
            );
        }
    });
}

void MainWindow::stopReminderThread()
{
    // 通知线程停止
    running_ = false;

    // 等待线程结束
    if (reminderThread_.joinable()) {
        reminderThread_.join();
    }
}

void MainWindow::playReminderSound()
{
    if (reminderSound_ && reminderSound_->source().isValid()) {
	qDebug() << "播放提醒音效";
        reminderSound_->play();
    } else {
        // 如果没有找到音频文件，就使用系统提示音兜底
        QApplication::beep();
    }
}

bool MainWindow::validateTaskForUi(const Task& task, int ignoreTaskId, QString& errorMessage) const
{
    if (task.name.trimmed().isEmpty()) {
        errorMessage = "任务名称不能为空！";
        return false;
    }

    if (!task.startTime.isValid()) {
        errorMessage = "开始时间无效！";
        return false;
    }

    QVector<Task> tasks = taskMgr_.getTasksByUser(username_);

    for (const Task& existingTask : tasks) {
        // 编辑任务时，跳过当前任务自身
        if (existingTask.id == ignoreTaskId) {
            continue;
        }

	QString existingStart = existingTask.startTime.toString("yyyy-MM-dd HH:mm");
	QString newStart = task.startTime.toString("yyyy-MM-dd HH:mm");
        // 规则 1：同一用户下，开始时间不能重复
        if (existingStart == newStart) {
            errorMessage = QString("已有任务使用相同开始时间：%1")
                .arg(task.startTime.toString("yyyy-MM-dd HH:mm"));
            return false;
        }

        // 规则 2：任务名称 + 开始时间不能重复
        if (existingTask.name == task.name &&
            existingStart == newStart) {
            errorMessage = "任务名称和开始时间不能与已有任务重复！";
            return false;
        }
    }

    return true;
}
