#include "user.h"
#include "utils.h"

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QByteArray>

namespace {

// 用户名编码成 Base64 (Base64 编码结果不包含普通空格)
QString encodeUsername(const QString& username)
{
    return QString::fromLatin1(username.toUtf8().toBase64());
}

// 用户名从 Base64 解码
QString decodeUsername(const QString& encodedUsername)
{
    QByteArray bytes = QByteArray::fromBase64(encodedUsername.toLatin1());
    return QString::fromUtf8(bytes);
}

}  //namespece

UserManager::UserManager()
{
}

// 注册新用户
bool UserManager::registerUser(const QString& username, const QString& password)
{
    QString name = utils::trim(username);

    if (name.isEmpty() || password.isEmpty()) {
        return false;
    }

    if (userExists(name)) {
        return false;
    }

    // 密码保存 SHA-256 哈希，不保存明文
    users_[name] = utils::sha256(password);

    return true;
}

bool UserManager::login(const QString& username, const QString& password)
{
    QString name = utils::trim(username);

    if (name.isEmpty() || password.isEmpty()) {
        return false;
    }

    if (!userExists(name)) {
        return false;
    }

    QString inputHash = utils::sha256(password);

    if (users_[name] == inputHash) {
        currentUser_ = name;
        return true;
    }

    // 密码不正确
    return false;
}

bool UserManager::userExists(const QString& username) const
{
    QString name = utils::trim(username);

    if (name.isEmpty()) {
        return false;
    }

    return users_.contains(name);  // 检查容器中是否存在该用户名
}

QString UserManager::currentUser() const
{
    return currentUser_;
}

bool UserManager::loadFromFile(const QString& filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);

    users_.clear();  //清空内存中原有的用户数据，准备加载文件中的用户信息

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();   // 逐行读取文件

        if (line.isEmpty()) {
            continue;
        }

        QStringList parts = line.split(' ', Qt::SkipEmptyParts);

        if (parts.size() != 2) {    // 字段数量应该都为2
            continue;
        }

        QString username = decodeUsername(parts[0].trimmed());
        QString passwordHash = parts[1].trimmed();

        username = utils::trim(username);

        if (!username.isEmpty() && !passwordHash.isEmpty()) {
            users_[username] = passwordHash;
        }
    }

    file.close();    // 关闭文件

    return true;
}

// 将用户数据保存到文件
bool UserManager::saveToFile(const QString& filename) const
{
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);

    // 遍历所有用户
    for (auto it = users_.begin(); it != users_.end(); ++it) {
        QString encodedUsername = encodeUsername(it.key());
        QString passwordHash = it.value();

        out << encodedUsername << " " << passwordHash << "\n";   //每个用户保存为一行：Base64用户名 密码哈希
    }

    file.close();

    return true;
}

