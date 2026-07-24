#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QString>

#include "storage.h"
#include "user.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget* parent = nullptr);
    ~LoginDialog();

    QString username() const;

private slots:
    void on_loginBtn_clicked();
    void on_registerBtn_clicked();
    void on_passwordEdit_returnPressed();
    void on_usernameEdit_returnPressed();

private:
    Ui::LoginDialog* ui;

    Storage storage_;
    UserManager userMgr_;

    QString username_;
};

#endif // LOGINDIALOG_H
