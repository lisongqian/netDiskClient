#include <iostream>
#include <map>
#include <QApplication>
#include <QTextCodec>
#include <QFile>
#include <QObject>
#include <QUuid>
#include <fstream>
#include <direct.h>
#include <QFontDatabase>
#include <windows.h>
#include <dbghelp.h>
#include "log/log.h"
#include "common.h"
#include "config.h"
#include "MainWindow.h"
#include "LoginDialog.h"

#define LOG_LEVEL alertLevel::E_INFO
using std::string;
using std::cout;
using std::map;
using std::endl;

Config g_config;

LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException)
{
    //创建 Dump 文件
    HANDLE hDumpFile = CreateFile(L"crash.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDumpFile != INVALID_HANDLE_VALUE)
    {
        //Dump信息
        MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
        dumpInfo.ExceptionPointers = pException;
        dumpInfo.ThreadId = GetCurrentThreadId();
        dumpInfo.ClientPointers = TRUE;

        //写入Dump文件内容
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char **argv) {
    // 1. 创建缓存文件目录
    if (0 != access("tmp", 0)) {
        mkdir("tmp");
    }
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);//注冊异常捕获函数
    // 2. 接收参数
    g_config.ParseArg(argc, argv);
    Log::instance()->Init(g_config.buff_size);

    //3. 加载编码格式和样式表
    QApplication app(argc, argv);
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");//或者"GBK",不分大小写
    QTextCodec::setCodecForLocale(codec);
    int localFont = QFontDatabase::addApplicationFont(":/PingFang.ttf");
    QString PingFangSC = QFontDatabase::applicationFontFamilies(localFont).at(0);
    cout << PingFangSC.toLocal8Bit().data() << endl;
    QFont font(PingFangSC, 10);
    QApplication::setFont(font);
    if (QFile qss(":/common.qss");qss.open(QFile::ReadOnly)) {
        app.setStyleSheet(qss.readAll());
        qss.close();
    }

    //4. 启动窗口
    static MainWindow mainWindow;
    LoginDialog dlg;

    // 5. 登录窗口显示判断，打开主窗口
    if (std::ifstream in(g_config.login_cache_path);in.is_open()) {
        string username, password;
        getline(in, username);
        getline(in, password);
        LOG_INFO("%s,%s", username.c_str(), password.c_str())
        bool flag = dlg.login(username, password);
        if (flag) {
            mainWindow.slot_show_myself();
        }
        else {
            mainWindow.addConnection(&dlg);
            dlg.show();
        }
    }
    else {
        LOG_INFO("no login cache file")
        mainWindow.addConnection(&dlg);
        dlg.show();
    }
    return QApplication::exec(); //应用程序运行
}
