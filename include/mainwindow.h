#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QVector>

#include "TaskManager.h"

class QTableWidget;
class QPushButton;
class QComboBox;
class QLabel;
class QTimer;

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
    QTimer* reminderTimer_;
};

#endif // MAINWINDOW_H
