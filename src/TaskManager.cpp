#include "TaskManager.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>

// 构造函数
TaskManager::TaskManager()
{
}

// 添加任务
bool TaskManager::addTask(const Task& task)
{
    // 任务名称不能为空
    if (task.name.trimmed().isEmpty()) {
        return false;
    }

    // 所属用户不能为空
    if (task.owner.trimmed().isEmpty()) {
        return false;
    }

    // 判断是否重复
    if (isDuplicateTask(task)) {
        return false;
    }

    // 创建一个新的任务对象
    Task newTask = task;

    // 如果外部没有设置 id，则自动生成一个 id
    if (newTask.id <= 0) {
        newTask.id = generateNextId();
    }

    // 添加到任务列表中
    tasks_.append(newTask);

    return true;
}

// 根据任务 id 删除任务
bool TaskManager::deleteTask(int taskId)
{
    // 遍历任务列表
    for (int i = 0; i < tasks_.size(); ++i) {
        // 找到 id 相同的任务
        if (tasks_[i].id == taskId) {
            // 从列表中删除该任务
            tasks_.removeAt(i);
            return true;
        }
    }

    // 没有找到对应 id 的任务
    return false;
}

// 获取某个用户的所有任务
QVector<Task> TaskManager::getTasksByUser(const QString& username) const
{
    QVector<Task> result;

    // 遍历所有任务
    for (const Task& task : tasks_) {
        // 只返回属于该用户的任务
        if (task.owner == username) {
            result.append(task);
        }
    }

    return result;
}

// 获取所有任务
QVector<Task> TaskManager::getAllTasks() const
{
    return tasks_;
}

// 生成新的任务 id
int TaskManager::generateNextId() const
{
    int maxId = 0;

    // 找出当前已有任务中的最大 id
    for (const Task& task : tasks_) {
        if (task.id > maxId) {
            maxId = task.id;
        }
    }

    // 新任务 id = 当前最大 id + 1
    return maxId + 1;
}

// 判断任务是否重复
bool TaskManager::isDuplicateTask(const Task& task) const
{
    for (const Task& existingTask : tasks_) {
        // 同一个用户、同一个任务名、同一个开始时间，就是重复任务
        if (existingTask.owner == task.owner &&
            existingTask.name == task.name &&
            existingTask.startTime == task.startTime) {
            return true;
        }
    }

    return false;
}

void TaskManager::clear()
{
    tasks_.clear();
}

bool TaskManager::loadFromFile(const QString& filename)
{
    QFile file(filename);

    // 文件不存在说明当前用户还没有任务，不算错误
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

    QVector<Task> loadedTasks;
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

        if (!task.name.trimmed().isEmpty() && !task.owner.trimmed().isEmpty()) {
            loadedTasks.append(task);
        }
    }

    tasks_ = loadedTasks;
    return true;
}

bool TaskManager::saveToFile(const QString& filename) const
{
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QJsonArray array;

    for (const Task& task : tasks_) {
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
