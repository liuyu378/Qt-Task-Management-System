#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QVector>

#include "TaskManager.h"
#include <thread>
#include <atomic>
#include <QSet>

class QTableWidget;
class QPushButton;
class QComboBox;
class QLabel;
class QSoundEffect;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString& username, QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onAddTask();
    void onEditTask();
    void onDeleteTask();
    void onRefresh();
    void onFilterChanged();    // 筛选条件变化槽函数：当筛选下拉框变化时重新加载任务列表
    void checkReminders();     // 任务提醒检查槽函数：由后台线程定时触发，检查是否有到点提醒的任务
    void onLogout();
    void onAbout();    // 显示软件版本和功能说明
    void playReminderSound();   // 播放任务提醒声音
protected:
    void closeEvent(QCloseEvent* event) override;    // 窗口关闭事件：关闭窗口前保存任务

private:
    void setupUiExtra();
    void loadTasks();
    void refreshTable(const QVector<Task>& tasks);
    int currentSelectedTaskId() const;

    // 获取当前用户的任务文件路径
    QString taskFilePath() const;

    // 程序进入主界面时，从文件加载任务到内存
    bool loadTasksFromFile();

    // 添加 / 编辑 / 删除任务后，自动保存任务到文件
    bool saveTasksToFile() const;

    // 启动后台提醒线程
    void startReminderThread();

    // 停止后台提醒线程
    void stopReminderThread();
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
};

#endif // MAINWINDOW_H
