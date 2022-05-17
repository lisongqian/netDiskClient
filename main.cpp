#include <iostream>
#include <map>
#include <QApplication>
#include <QTextCodec>
#include <QFile>
#include <QObject>
#include <QUuid>
#include <fstream>
#include <direct.h>
#include "log/log.h"
#include "common.h"
#include "config.h"
#include "MainWindow.h"
#include "LoginDialog.h"

using std::string;
using std::cout;
using std::map;
using std::endl;

Config g_config;

int main(int argc, char **argv) {
    // 1. 接收参数
    g_config.ParseArg(argc, argv);
    Log::instance()->Init(g_config.buff_size);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QApplication app(argc, argv);
    if(QFile qss(":/main.qss");qss.open(QFile::ReadOnly))
    {
        app.setStyleSheet(qss.readAll());
        qss.close();
    }
//    string uuid = QUuid::createUuid().toString().remove("{").remove("}").remove("-").toStdString();
//    cout << uuid << endl;
    static MainWindow mainWindow;
    LoginDialog dlg;
    if (0 != access("tmp", 0)) {
        mkdir("tmp");
    }
    if (std::ifstream in(g_config.login_cache_path);in.is_open()) {
        string username, password;
        getline(in, username);
        getline(in, password);
        LOG_INFO("%s,%s", username.c_str(), password.c_str())
        bool flag = dlg.login(username, password);
        if (flag) {
            mainWindow.show();
        }
        else {
            mainWindow.add_connection(&dlg);
            dlg.show();
        }
    }
    else {
        LOG_INFO("open file failed")
        mainWindow.add_connection(&dlg);
        dlg.show();
    }
    return QApplication::exec(); //应用程序运行

}
