#include "mainwindow.h"
#include "taskdialog.h"

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QTimer>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QDate>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>

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
      reminderTimer_(nullptr)
{
    // 先从本地文件加载任务到 taskMgr_ 内存中
    // 这一步就是“用户登录后，从文件加载任务列表，保存到内存”
    loadTasksFromFile();

    // 初始化主界面
    setupUiExtra();

    // 把内存中的任务显示到表格
    loadTasks();

    // 定时检查任务提醒
    reminderTimer_ = new QTimer(this);
    connect(reminderTimer_, &QTimer::timeout, this, &MainWindow::checkReminders);
    reminderTimer_->start(60000);
}

MainWindow::~MainWindow()
{
    // 窗口销毁前再保存一次，防止数据丢失
    saveTasksToFile();
}

// 获取当前用户的任务文件路径
// 每个用户单独一个任务文件
QString MainWindow::taskFilePath() const
{
    QDir dir("./data");

    // 如果 data 目录不存在，就创建
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // 用户名可能包含空格、中文或特殊字符
    // 所以用 Base64Url 编码成安全文件名
    QByteArray encoded = username_.toUtf8().toBase64(
        QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals
    );

    QString safeUsername = QString::fromLatin1(encoded);

    return dir.filePath("tasks_" + safeUsername + ".dat");
}

// 从本地文件加载任务
bool MainWindow::loadTasksFromFile()
{
    QFile file(taskFilePath());

    // 文件不存在说明该用户还没有任务，不算错误
    if (!file.exists()) {
        return true;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError || !doc.isArray()) {
        return false;
    }

    QJsonArray array = doc.array();

    for (const QJsonValue& value : array) {
        if (!value.isObject()) {
            continue;
        }

        QJsonObject obj = value.toObject();

        Task task;

        task.id = obj.value("id").toInt();
        task.name = obj.value("name").toString();
        task.startTime = QDateTime::fromString(
            obj.value("startTime").toString(),
            Qt::ISODate
        );
        task.priority = obj.value("priority").toString("中");
        task.category = obj.value("category").toString("生活");

        QString reminderText = obj.value("reminderTime").toString();
        if (!reminderText.isEmpty()) {
            task.reminderTime = QDateTime::fromString(reminderText, Qt::ISODate);
        }

        task.owner = obj.value("owner").toString();

        // 只加载当前用户自己的任务
        if (task.owner == username_) {
            taskMgr_.addTask(task);
        }
    }

    return true;
}

// 保存当前用户任务到本地文件
bool MainWindow::saveTasksToFile() const
{
    QFile file(taskFilePath());

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QJsonArray array;

    QVector<Task> tasks = taskMgr_.getTasksByUser(username_);

    for (const Task& task : tasks) {
        QJsonObject obj;

        obj["id"] = task.id;
        obj["name"] = task.name;
        obj["startTime"] = task.startTime.toString(Qt::ISODate);
        obj["priority"] = task.priority;
        obj["category"] = task.category;

        if (task.reminderTime.isValid()) {
            obj["reminderTime"] = task.reminderTime.toString(Qt::ISODate);
        } else {
            obj["reminderTime"] = "";
        }

        obj["owner"] = task.owner;

        array.append(obj);
    }

    QJsonDocument doc(array);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
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

        if (taskMgr_.addTask(task)) {
            // 每添加一个任务，立即保存到文件
            saveTasksToFile();

            loadTasks();
        } else {
            QMessageBox::warning(this, "添加失败", "任务名称不能为空，或者任务重复。");
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

        taskMgr_.deleteTask(oldTask.id);

        if (!taskMgr_.addTask(newTask)) {
            taskMgr_.addTask(oldTask);
            QMessageBox::warning(this, "修改失败", "任务名称不能为空，或者任务重复。");
            return;
        }

        // 每次编辑完成后，立即保存到文件
        saveTasksToFile();

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
            // 每次删除完成后，立即保存到文件
            saveTasksToFile();

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
        if (task.reminderTime.isValid() &&
            task.reminderTime <= now &&
            task.startTime >= now) {
            QMessageBox::information(
                this,
                "任务提醒",
                QString("任务：%1\n开始时间：%2\n优先级：%3\n分类：%4")
                    .arg(task.name)
                    .arg(task.startTime.toString("yyyy-MM-dd HH:mm"))
                    .arg(task.priority)
                    .arg(task.category)
            );
        }
    }
}

void MainWindow::onLogout()
{
    saveTasksToFile();
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
    saveTasksToFile();
    event->accept();
}
