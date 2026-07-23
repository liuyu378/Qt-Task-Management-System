#ifndef STORAGE_H
#define STORAGE_H

// 文件存储管理：统一负责用户数据和任务数据的文件路径管理与读写入口

#include <QString>
#include "user.h"
#include "TaskManager.h"

class Storage {
public:
    // dataDir 表示数据保存目录，默认保存到当前程序运行目录下的 data 文件夹
    explicit Storage(const QString& dataDir = "./data");

    // 初始化存储目录，如果 data 目录不存在则创建
    bool init();

    // 加载 / 保存所有用户信息
    bool loadUsers(UserManager& um);
    bool saveUsers(const UserManager& um);

    // 加载 / 保存某个用户的任务信息
    bool loadTasks(const QString& username, UserManager& tm);
    bool saveTasks(const QString& username, const UserManager& tm);

private:
    // 获取用户数据文件路径，例如：./data/users.dat
    QString getUserFilePath() const;

    // 获取某个用户的任务数据文件路径，例如：./data/tasks_xxx.dat
    QString getTaskFilePath(const QString& username) const;

    // 将用户名编码成适合文件名使用的字符串
    // 避免用户名中的空格、斜杠、中文等字符影响文件路径
    QString encodeFileName(const QString& text) const;

private:
    QString dataDir_;
};

#endif // STORAGE_H
