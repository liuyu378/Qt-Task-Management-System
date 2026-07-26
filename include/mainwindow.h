#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QVector>
#include <QSet>
#include <QCheckBox>

#include <thread>
#include <atomic>

#include "TaskManager.h"
#include "storage.h"

class QTableWidget;
class QPushButton;
class QComboBox;
class QLabel;
class QSoundEffect;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // 构造函数：创建主窗口，并接收当前登录用户名
    explicit MainWindow(const QString& username, QWidget* parent = nullptr);

    // 析构函数：窗口销毁时停止后台提醒线程
    ~MainWindow();

signals:
    // 退出登录信号：通知 main.cpp 返回登录界面
    void logoutRequested();
private slots:
    // 添加任务：打开 TaskDialog，获取用户输入的新任务
    void onAddTask();

    // 编辑任务：编辑当前表格中选中的任务
    void onEditTask();

    // 删除任务：根据当前选中任务的 ID 删除任务
    void onDeleteTask();

    // 刷新任务列表：重新根据筛选条件显示任务
    void onRefresh();

    // 筛选条件或排序选项变化时刷新任务列表
    void onFilterChanged();

    // 任务提醒检查：由后台线程周期性触发
    void checkReminders();

    // 退出当前用户界面
    void onLogout();

    // 显示软件版本和功能说明
    void onAbout();

protected:
    // 窗口关闭事件：关闭窗口前保存任务并停止提醒线程
    void closeEvent(QCloseEvent* event) override;

private:
    // 初始化主窗口界面控件和信号槽连接
    void setupUiExtra();

    // 根据当前筛选条件刷新任务列表
    void loadTasks();

    // 将任务数据显示到 QTableWidget 表格中
    void refreshTable(const QVector<Task>& tasks);

    // 获取当前表格选中行的任务 ID
    // 如果没有选中任务，返回 -1
    int currentSelectedTaskId() const;

    // 在界面层检查任务是否违反规则
    // ignoreTaskId 用于编辑任务时忽略当前任务自身
    bool validateTaskForUi(const Task& task,
                           int ignoreTaskId,
                           QString& errorMessage) const;

    // 启动后台提醒线程
    void startReminderThread();

    // 停止后台提醒线程
    void stopReminderThread();

    // 播放任务提醒声音
    void playReminderSound();

private:
    // 当前登录用户名
    QString username_;

    // 任务管理器：负责内存中的任务增删查和文件读写
    TaskManager taskMgr_;

    // 存储管理器：负责 data 目录和用户/任务文件路径
    Storage storage_;

    // 主界面任务表格
    QTableWidget* table_;

    // 功能按钮
    QPushButton* addButton_;
    QPushButton* editButton_;
    QPushButton* deleteButton_;
    QPushButton* refreshButton_;
    QPushButton* logoutButton_;
    QPushButton* aboutButton_;

    // 任务筛选下拉框：全部任务 / 今日任务 / 本月任务
    QComboBox* filterBox_;

    // 是否按开始时间排序
    QCheckBox* sortCheckBox_;

    // 当前用户显示标签
    QLabel* userLabel_;

    // 后台提醒线程
    std::thread reminderThread_;

    // 控制后台线程是否运行
    std::atomic<bool> running_;

    // 已经提醒过的任务 ID，用于防止重复弹窗
    QSet<int> remindedTaskIds_;

    // 提醒音效对象
    QSoundEffect* reminderSound_;
};

#endif // MAINWINDOW_H
