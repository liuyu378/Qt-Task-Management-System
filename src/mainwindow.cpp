#include "mainwindow.h"
#include "taskdialog.h"

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
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
#include <QSoundEffect>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QApplication>

#include <chrono>
#include <thread>
#include <algorithm>


// 主窗口构造函数
// username 是登录成功后传入的当前用户名
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
      sortCheckBox_(nullptr),
      userLabel_(nullptr),
      reminderSound_(nullptr),
      running_(false)
{
    // 初始化 data 目录
    storage_.init();

    // 用户登录后，从本地文件加载该用户任务到 TaskManager 内存中
    storage_.loadTasks(username_, taskMgr_);

    // 初始化主界面控件和信号槽
    setupUiExtra();

    // 初始化提醒音效
    reminderSound_ = new QSoundEffect(this);

    // 程序一般在 build 目录运行
    // ../resources/reminder_fixed.wav 指向项目根目录下的 resources/reminder_fixed.wav
    QString soundPath = "../resources/reminder_fixed.wav";

    if (QFile::exists(soundPath)) {
        QString absolutePath = QFileInfo(soundPath).absoluteFilePath();

        reminderSound_->setSource(QUrl::fromLocalFile(absolutePath));
        reminderSound_->setLoopCount(1);
        reminderSound_->setVolume(1.0);

        qDebug() << "提醒音效已设置:" << reminderSound_->source();
    } else {
        qDebug() << "提醒音频文件不存在:" << soundPath;
    }

    // 将内存中的任务显示到表格
    loadTasks();

    // 启动后台提醒线程
    startReminderThread();

    // 进入主界面后立即检查一次提醒
    checkReminders();
}


// 析构函数
MainWindow::~MainWindow()
{
    // 停止后台提醒线程
    stopReminderThread();
}


// 初始化主窗口界面
void MainWindow::setupUiExtra()
{
    setWindowTitle("MySchedule - " + username_);
    resize(900, 600);

    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    // 当前用户显示
    userLabel_ = new QLabel("当前用户：" + username_, this);

    // 筛选下拉框
    filterBox_ = new QComboBox(this);
    filterBox_->addItem("全部任务");
    filterBox_->addItem("今日任务");
    filterBox_->addItem("本月任务");

    // 是否按开始时间排序
    sortCheckBox_ = new QCheckBox("按开始时间排序", this);
    sortCheckBox_->setChecked(false);

    // 功能按钮
    addButton_ = new QPushButton("添加任务", this);
    editButton_ = new QPushButton("编辑任务", this);
    deleteButton_ = new QPushButton("删除任务", this);
    refreshButton_ = new QPushButton("刷新", this);
    aboutButton_ = new QPushButton("关于", this);
    logoutButton_ = new QPushButton("退出登录", this);

    // 任务表格
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

    // 顶部区域：当前用户 + 筛选 + 排序
    QHBoxLayout* topLayout = new QHBoxLayout;
    topLayout->addWidget(userLabel_);
    topLayout->addStretch();
    topLayout->addWidget(new QLabel("筛选：", this));
    topLayout->addWidget(filterBox_);
    topLayout->addWidget(sortCheckBox_);

    // 底部按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(addButton_);
    buttonLayout->addWidget(editButton_);
    buttonLayout->addWidget(deleteButton_);
    buttonLayout->addWidget(refreshButton_);
    buttonLayout->addStretch();
    buttonLayout->addWidget(aboutButton_);
    buttonLayout->addWidget(logoutButton_);

    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(table_);
    mainLayout->addLayout(buttonLayout);

    // 信号槽连接
    connect(addButton_, &QPushButton::clicked,
            this, &MainWindow::onAddTask);

    connect(editButton_, &QPushButton::clicked,
            this, &MainWindow::onEditTask);

    connect(deleteButton_, &QPushButton::clicked,
            this, &MainWindow::onDeleteTask);

    connect(refreshButton_, &QPushButton::clicked,
            this, &MainWindow::onRefresh);

    connect(logoutButton_, &QPushButton::clicked,
            this, &MainWindow::onLogout);

    connect(aboutButton_, &QPushButton::clicked,
            this, &MainWindow::onAbout);

    connect(filterBox_,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &MainWindow::onFilterChanged);

    connect(sortCheckBox_, &QCheckBox::toggled,
            this, &MainWindow::onFilterChanged);
}


// 根据筛选条件刷新任务列表
void MainWindow::loadTasks()
{
    QVector<Task> allTasks = taskMgr_.getTasksByUser(username_);
    QVector<Task> result;

    int filterIndex = filterBox_ ? filterBox_->currentIndex() : 0;
    QDate today = QDate::currentDate();

    for (const Task& task : allTasks) {
        if (filterIndex == 0) {
            // 全部任务
            result.append(task);
        } else if (filterIndex == 1) {
            // 今日任务
            if (task.startTime.date() == today) {
                result.append(task);
            }
        } else if (filterIndex == 2) {
            // 本月任务
            QDate d = task.startTime.date();

            if (d.year() == today.year() &&
                d.month() == today.month()) {
                result.append(task);
            }
        }
    }

    // 如果勾选“按开始时间排序”，则对当前显示结果排序
    // 排序只影响界面显示，不改变文件中的存储顺序
    if (sortCheckBox_ && sortCheckBox_->isChecked()) {
        std::sort(result.begin(), result.end(),
                  [](const Task& a, const Task& b) {
                      return a.startTime < b.startTime;
                  });
    }

    refreshTable(result);
}


// 将任务列表显示到表格
void MainWindow::refreshTable(const QVector<Task>& tasks)
{
    table_->setRowCount(tasks.size());

    for (int row = 0; row < tasks.size(); ++row) {
        const Task& t = tasks[row];

        table_->setItem(row, 0,
                        new QTableWidgetItem(QString::number(t.id)));

        table_->setItem(row, 1,
                        new QTableWidgetItem(t.name));

        table_->setItem(row, 2,
                        new QTableWidgetItem(t.startTime.toString("yyyy-MM-dd HH:mm")));

        table_->setItem(row, 3,
                        new QTableWidgetItem(t.priority));

        table_->setItem(row, 4,
                        new QTableWidgetItem(t.category));

        QString reminderText;
        if (t.reminderTime.isValid()) {
            reminderText = t.reminderTime.toString("yyyy-MM-dd HH:mm");
        } else {
            reminderText = "无";
        }

        table_->setItem(row, 5,
                        new QTableWidgetItem(reminderText));

        // 高优先级任务高亮显示
        if (t.priority == "高") {
            for (int col = 0; col < table_->columnCount(); ++col) {
                table_->item(row, col)->setBackground(Qt::yellow);
            }
        }
    }
}


// 获取当前选中任务的 ID
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


// 添加任务
void MainWindow::onAddTask()
{
    TaskDialog dlg(this);

    if (dlg.exec() == QDialog::Accepted) {
        Task task = dlg.getTask();

        // owner 由主窗口根据当前登录用户设置
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


// 编辑任务
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

        // 编辑任务时保持原来的 ID 和 owner
        newTask.id = oldTask.id;
        newTask.owner = username_;

        QString errorMessage;
        if (!validateTaskForUi(newTask, oldTask.id, errorMessage)) {
            QMessageBox::warning(this, "修改失败", errorMessage);
            return;
        }

        // 当前 TaskManager 没有 updateTask，因此用先删后加的方式更新
        taskMgr_.deleteTask(oldTask.id);

        if (!taskMgr_.addTask(newTask)) {
            // 如果修改失败，恢复旧任务
            taskMgr_.addTask(oldTask);

            QMessageBox::warning(this, "修改失败", "任务修改失败，请检查任务信息。");
            return;
        }

        // 如果任务之前已经提醒过，编辑后允许再次提醒
        remindedTaskIds_.remove(oldTask.id);

        storage_.saveTasks(username_, taskMgr_);
        loadTasks();
    }
}


// 删除任务
void MainWindow::onDeleteTask()
{
    int taskId = currentSelectedTaskId();

    if (taskId <= 0) {
        QMessageBox::information(this, "提示", "请先选择一个任务。");
        return;
    }

    int ret = QMessageBox::question(
        this,
        "确认删除",
        "确定要删除这个任务吗？"
    );

    if (ret == QMessageBox::Yes) {
        if (taskMgr_.deleteTask(taskId)) {
            remindedTaskIds_.remove(taskId);

            storage_.saveTasks(username_, taskMgr_);
            loadTasks();
        } else {
            QMessageBox::warning(this, "删除失败", "没有找到该任务。");
        }
    }
}


// 刷新任务列表
void MainWindow::onRefresh()
{
    loadTasks();
}


// 筛选条件或排序选项变化
void MainWindow::onFilterChanged()
{
    loadTasks();
}


// 检查任务提醒
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
        if (task.startTime < now) {
            continue;
            }

            // 控制台打印提醒
            qDebug() << "任务提醒："
                     << "任务名称:" << task.name
                     << "开始时间:" << task.startTime.toString("yyyy-MM-dd HH:mm")
                     << "提醒时间:" << task.reminderTime.toString("yyyy-MM-dd HH:mm")
                     << "优先级:" << task.priority
                     << "分类:" << task.category;

            // 播放提醒音效
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


// 退出登录
void MainWindow::onLogout()
{
    int ret = QMessageBox::question(
        this,
        "退出登录",
        "确定要退出当前用户吗？"
    );

    if (ret == QMessageBox::Yes) {
        storage_.saveTasks(username_, taskMgr_);
	// 发出信号，通知 main.cpp 重新显示登录窗口
        emit logoutRequested();
        close();
    }
}


// 关于窗口
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
        "- 可选按开始时间排序\n"
        "- 后台任务提醒\n"
        "- 音乐提醒\n"
        "- 语音录入任务"
    );
}


// 窗口关闭事件
void MainWindow::closeEvent(QCloseEvent* event)
{
    storage_.saveTasks(username_, taskMgr_);
    stopReminderThread();

    event->accept();
}


// 启动后台提醒线程
void MainWindow::startReminderThread()
{
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

            // 后台线程不能直接操作 Qt UI
            // 因此投递到主线程中执行 checkReminders()
            QMetaObject::invokeMethod(
                this,
                "checkReminders",
                Qt::QueuedConnection
            );
        }
    });
}


// 停止后台提醒线程
void MainWindow::stopReminderThread()
{
    running_ = false;

    if (reminderThread_.joinable()) {
        reminderThread_.join();
    }
}


// 播放任务提醒声音
void MainWindow::playReminderSound()
{
    if (reminderSound_ && reminderSound_->source().isValid()) {
        qDebug() << "播放提醒音效";
	// 防止上一次音效还没结束时再次叠加播放
        if (reminderSound_->isPlaying()) {
            reminderSound_->stop();
        }
        // 明确设置只播放一次
        reminderSound_->setLoopCount(1);
        // 播放一次提醒音效
        reminderSound_->play();
    } else {
        // 如果音频文件加载失败，使用系统蜂鸣声兜底
        QApplication::beep();
    }
}


// GUI 层任务校验
// ignoreTaskId 用于编辑任务时忽略当前任务自身
bool MainWindow::validateTaskForUi(const Task& task,
                                   int ignoreTaskId,
                                   QString& errorMessage) const
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

    // 新任务开始时间按分钟比较
    QString newStart =
        task.startTime.toString("yyyy-MM-dd HH:mm");

    for (const Task& existingTask : tasks) {
        // 编辑任务时，跳过当前任务自身
        if (existingTask.id == ignoreTaskId) {
            continue;
        }

        QString existingStart =
            existingTask.startTime.toString("yyyy-MM-dd HH:mm");

        // 规则 1：同一用户下，开始时间不能重复
        if (existingStart == newStart) {
            errorMessage = QString("已有任务使用相同开始时间：%1")
                .arg(newStart);
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
