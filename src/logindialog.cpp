#include "logindialog.h"
#include "ui_logindialog.h"

#include <QMessageBox>

// 登录窗口构造函数：初始化界面、加载已有用户数据，并设置窗口标题与尺寸
LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    storage_.init();
    storage_.loadUsers(userMgr_);

    setWindowTitle("MySchedule - 登录");
    setFixedSize(380, 260);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

// 返回当前已登录的用户名，供主窗口使用
QString LoginDialog::username() const
{
    return username_;
}

// 登录按钮点击处理：读取输入内容，检查用户名和密码是否有效；若用户不存在则询问是否注册
void LoginDialog::on_loginBtn_clicked()
{
    QString user = ui->usernameEdit->text().trimmed();
    QString pass = ui->passwordEdit->text();

    if (user.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "错误", "用户名和密码不能为空！");
        return;
    }

    // 注意：这里直接使用 QString，不要转 std::string
    if (!userMgr_.userExists(user)) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "用户不存在",
            QString("用户 '%1' 不存在，是否注册？").arg(user),
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            if (userMgr_.registerUser(user, pass)) {
                storage_.saveUsers(userMgr_);
                username_ = user;
                accept();
            } else {
                QMessageBox::warning(this, "注册失败", "注册用户失败！");
            }
        }

        return;
    }

    if (userMgr_.login(user, pass)) {
        username_ = user;
        accept();
    } else {
        QMessageBox::warning(this, "登录失败", "密码不正确！");
    }
}

// 注册按钮点击处理：把新用户写入用户管理器，并保存到磁盘，随后直接进入登录状态
void LoginDialog::on_registerBtn_clicked()
{
    QString user = ui->usernameEdit->text().trimmed();
    QString pass = ui->passwordEdit->text();

    if (user.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "错误", "用户名和密码不能为空！");
        return;
    }

    // 注意：这里也直接传 QString
    if (userMgr_.registerUser(user, pass)) {
        storage_.saveUsers(userMgr_);
        QMessageBox::information(this, "成功", "用户注册成功！");
        username_ = user;
        accept();
    } else {
        QMessageBox::warning(this, "注册失败", "用户名可能已经存在！");
    }
}

void LoginDialog::on_passwordEdit_returnPressed()
{
    on_loginBtn_clicked();
}

void LoginDialog::on_usernameEdit_returnPressed()
{
    ui->passwordEdit->setFocus();
}
