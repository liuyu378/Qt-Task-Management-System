#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QDateTime>

namespace utils {

// 密码哈希
QString sha256(const QString& input);

// 时间格式化
QString dateTimeToString(const QDateTime& dateTime);
QDateTime stringToDateTime(const QString& str);

// 字符串处理
QString trim(const QString& str);

}

#endif // UTILS_H
