#include <QApplication>
#include <QDialog>
#include <QEventLoop>

#include "logindialog.h"
#include "mainwindow.h"

// 程序主入口：先创建登录窗口，用户登录成功后再进入主界面；用户注销或关闭时可重新进入登录流程
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 防止最后一个窗口关闭时 QApplication 自动退出
    QApplication::setQuitOnLastWindowClosed(false);

    while (true) {
        LoginDialog loginDialog;

        // 如果用户关闭登录窗口，程序退出
        if (loginDialog.exec() != QDialog::Accepted) {
            break;
        }

        bool logout = false;

        MainWindow* mainWindow = new MainWindow(loginDialog.username());
        mainWindow->setAttribute(Qt::WA_DeleteOnClose);

        // 如果主窗口发出 logoutRequested，说明用户选择返回登录界面
        QObject::connect(mainWindow, &MainWindow::logoutRequested,
                         [&logout]() {
                             logout = true;
                         });

        QEventLoop loop;

        // 主窗口关闭后退出当前局部事件循环
        QObject::connect(mainWindow, &QObject::destroyed,
                         &loop, &QEventLoop::quit);

        mainWindow->show();

        // 等待主窗口关闭
        loop.exec();

        // 如果是注销，就重新回到登录界面
        if (logout) {
            continue;
        }

        // 如果是普通关闭窗口，则退出程序
        break;
    }

    return 0;
}
