#include <iostream>
#include <map>
#include <QApplication>
#include <QTextCodec>
#include <QFile>
#include <QObject>
#include "log/log.h"
#include "common.h"
#include "config.h"
#include "MainWindow.h"
#include "LoginDialog.h"

using std::string;
using std::cout;
using std::map;
using std::endl;

int main(int argc, char **argv) {
    // 1. 接收参数
    Config config;
    config.ParseArg(argc, argv);
    Log::instance()->Init(config.buff_size);
    LOG_INFO("HELLO WINDOWS")
    map<string, string> m;
    m["data"] = "123";
    LOG_INFO(Map2String(m).c_str())
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QApplication app(argc, argv);
    QFile qss(":/main.qss");
    qss.open(QFile::ReadOnly);
    app.setStyleSheet(qss.readAll());
    qss.close();
    static MainWindow mainWindow;
    LoginDialog dlg;
    mainWindow.add_connection(&dlg);
    dlg.show();
    return QApplication::exec(); //应用程序运行

}
