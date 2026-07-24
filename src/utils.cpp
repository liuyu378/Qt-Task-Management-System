#include "utils.h"
#include <QCryptographicHash>  // Qt 提供的加密哈希类，用于计算 SHA-256

namespace utils {

QString sha256(const QString& input)
{
    QByteArray hash = QCryptographicHash::hash(  
        input.toUtf8(),     //把 QString 转换成 UTF-8 编码的 QByteArray
        QCryptographicHash::Sha256
    );

    return QString(hash.toHex());   //转换成十六进制
}

QString dateTimeToString(const QDateTime& dateTime)  //把 QDateTime 转成字符串
{
    return dateTime.toString("yyyy-MM-dd HH:mm");
}

QDateTime stringToDateTime(const QString& str)
{
    return QDateTime::fromString(str, "yyyy-MM-dd HH:mm");
}

QString trim(const QString& str)   //去掉字符串开头和结尾的空白字符
{
    return str.trimmed();
}

} //namespace utils
