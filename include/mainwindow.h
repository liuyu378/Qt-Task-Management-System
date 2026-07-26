#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QVector>

#include "TaskManager.h"
#include <thread>
#include <atomic>
#include <QSet>
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
    explicit MainWindow(const QString& username, QWidget* parent = nullptr);    //构造函数
    ~MainWindow();

private slots:
    void onAddTask();
    void onEditTask();
    void onDeleteTask();
    void onRefresh();          // 刷新任务列表：重新根据筛选条件显示任务
    void onFilterChanged();    // 筛选条件变化槽函数：当筛选下拉框变化时重新加载任务列表
    void checkReminders();     // 任务提醒检查槽函数：由后台线程定时触发，检查是否有到点提醒的任务
    void onLogout();
    void onAbout();    // 显示软件版本和功能说明
protected:
    void closeEvent(QCloseEvent* event) override;    // 窗口关闭事件：关闭窗口前保存任务

private:
    void setupUiExtra();    // 初始化主窗口界面控件和信号槽连接
    void loadTasks();
    void refreshTable(const QVector<Task>& tasks);   // 将任务数据显示到 QTableWidget 表格中
    int currentSelectedTaskId() const;     // 获取当前表格选中行的任务 ID

    // 在界面层检查任务是否违反规则
    // ignoreTaskId 用于编辑任务时忽略当前任务自身
    bool validateTaskForUi(const Task& task, int ignoreTaskId, QString& errorMessage) const;
    // 启动后台提醒线程
    void startReminderThread();

    // 停止后台提醒线程
    void stopReminderThread();
    void playReminderSound();   // 播放任务提醒声音
private:
    QString username_;
    TaskManager taskMgr_;

    QTableWidget* table_;
    QPushButton* addButton_;
    QPushButton* editButton_;
    QPushButton* deleteButton_;
    QPushButton* refreshButton_;
    QPushButton* logoutButton_;
    QPushButton* aboutButton_;
    QComboBox* filterBox_;
    QLabel* userLabel_;

    // 后台提醒线程
    std::thread reminderThread_;

    // 控制后台线程是否运行
    std::atomic<bool> running_;

    // 已经提醒过的任务 ID，防止重复弹窗
    QSet<int> remindedTaskIds_;

    // 提醒音效对象
    QSoundEffect* reminderSound_;

    Storage storage_;
};

#endif // MAINWINDOW_H
