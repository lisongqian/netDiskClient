//
// Created by lisongqian on 2022/5/13.
//

//#include <QAbstractItemView>
#include <QMessageBox>
#include "LoginDialog.h"
#include "request/HTTPRequest.h"
#include "rapidjson/document.h"

using namespace rapidjson;
using std::string;
using std::map;

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LoginDialog) {
    ui->setupUi(this);
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::slot_login);
}

void LoginDialog::slot_login() {
    string username = ui->userName->text().toStdString();
    string password = ui->password->text().toStdString();
    if (!username.empty() && !password.empty()) {
        map<string, string> data;
        string res;
        data["username"] = username;
        data["password"] = password;
        if(username=="lisongqian"&&password=="123123")
        {
            emit open_main_window();
            this->close();
        }
        else
            QMessageBox::information(this, "错误", "用户名或密码不正确，请重新输入");
        return ;
        HTTPRequest *req = new HTTPRequest("127.0.0.1", 23450);
        bool flag = req->Post("/login", data, res);
        if (flag) {
            Document document;
            document.Parse(res.c_str());
            if (document["status"].GetInt() == 200 && document["code"].GetInt() == 1) {
                emit open_main_window();
            } else {
                QMessageBox::information(this, "错误", "用户名或密码不正确，请重新输入");
            }
        }
        QMessageBox::information(this, "错误", "服务器或网络错误，请重试");
    } else {
        QMessageBox::information(this, "错误", "请输入用户名和密码");
    }
}
