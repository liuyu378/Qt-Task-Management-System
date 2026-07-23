// 存储实现

#include "storage.h"
#include <QDir>
#include <QByteArray>

Storage::Storage(const QString& dataDir)
    : dataDir_(dataDir)
{
}

bool Storage::init()
{
    QDir dir;

    // 如果目录已经存在，直接返回 true
    if (dir.exists(dataDir_)) {
        return true;
    }

    // mkpath 可以创建多级目录，例如 ./data/user/files
    return dir.mkpath(dataDir_);
}

bool Storage::loadUsers(UserManager& um)
{
    // 用户信息统一保存在 users.dat 中
    return um.loadFromFile(getUserFilePath());
}

bool Storage::saveUsers(const UserManager& um)
{
    // 保存用户信息到 users.dat
    return um.saveToFile(getUserFilePath());
}

bool Storage::loadTasks(const QString& username, UserManager& tm)
{
    // 每个用户使用独立任务文件
    return tm.loadFromFile(getTaskFilePath(username));
}

bool Storage::saveTasks(const QString& username, const UserManager& tm)
{
    // 将该用户的任务保存到对应文件
    return tm.saveToFile(getTaskFilePath(username));
}

QString Storage::getUserFilePath() const
{
    QDir dir(dataDir_);

    // 返回类似：./data/users.dat
    return dir.filePath("users.dat");
}

QString Storage::getTaskFilePath(const QString& username) const
{
    QDir dir(dataDir_);

    // 用户名不能直接拼到文件名里，否则空格、/、中文等字符可能导致路径问题
    QString encodedUsername = encodeFileName(username);

    // 返回类似：./data/tasks_emhhbmcgc2Fu.dat
    return dir.filePath("tasks_" + encodedUsername + ".dat");
}

QString Storage::encodeFileName(const QString& text) const
{
    // 使用 UTF-8 + Base64Url 编码，生成适合文件名使用的字符串
    // Base64UrlEncoding 会避免普通 Base64 中的 '/'、'+' 等字符
    QByteArray encoded = text.toUtf8().toBase64(
        QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals
    );

    return QString::fromLatin1(encoded);
}
