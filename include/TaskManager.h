#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "task.h"

#include <QVector>
#include <QString>

// TaskManager 负责管理任务列表
class TaskManager
{
public:
    TaskManager();

    // 添加任务
    bool addTask(const Task& task);

    // 根据任务 id 删除任务
    bool deleteTask(int taskId);

    // 获取某个用户的所有任务
    QVector<Task> getTasksByUser(const QString& username) const;

    // 获取所有任务
    QVector<Task> getAllTasks() const;

    // 清空当前内存中的任务列表
    void clear();

    // 从文件加载任务数据
    bool loadFromFile(const QString& filename);

    // 保存任务数据到文件
    bool saveToFile(const QString& filename) const;
private:
    // 生成下一个任务 id
    int generateNextId() const;

    // 判断是否存在重复任务
    bool isDuplicateTask(const Task& task) const;

private:
    QVector<Task> tasks_; // 保存所有任务
};

#endif // TASKMANAGER_H
