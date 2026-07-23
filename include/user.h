#ifndef USER_H
#define USER_H

// 用户管理类：注册、登录、密码哈希验证、文件读写

#include <QString>
#include <QMap>

class UserManager {
public:
    UserManager();

    // 注册用户
    bool registerUser(const QString& username, const QString& password);

    // 用户登录
    bool login(const QString& username, const QString& password);

    // 判断用户是否存在
    bool userExists(const QString& username) const;

    // 从文件加载用户数据
    bool loadFromFile(const QString& filename);

    // 保存用户数据到文件
    bool saveToFile(const QString& filename) const;

    // 获取当前登录用户
    QString currentUser() const;

private:
    // 保存用户信息
    // key：用户名
    // value：密码哈希值
    QMap<QString, QString> users_;

    // 当前登录用户名
    QString currentUser_;
};

#endif // USER_H
