//
// Created by lisongqian on 2022/5/13.
//

//#include <QAbstractItemView>
#include <QMessageBox>
#include <fstream>
#include <memory>
#include "LoginDialog.h"
#include "request/HTTPRequest.h"
#include "rapidjson/document.h"
#include "config.h"
#include "log/log.h"

using namespace rapidjson;
using std::string;
using std::map;


LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent), ui(std::make_shared<Ui::LoginDialog>()), m_ip("127.0.0.1"),
                                            m_port(23450) {
    ui->setupUi(this);
//    m_ip = "127.0.0.1";
//    m_ip = "192.168.229.129";
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::slot_login);
    connect(ui->registerButton, &QPushButton::clicked, this, &LoginDialog::slot_register);
    connect(ui->resetButton, &QPushButton::clicked, this, &LoginDialog::slot_reset);
}

void LoginDialog::slot_login() {
    string username = ui->userName->text().toStdString();
    string password = ui->password->text().toStdString();
    if (!username.empty() && !password.empty()) {
        string res;
        bool flag = login(username, password, true);
        if (flag) {
            emit open_main_window();
            this->close();
        }
    }
    else {
        QMessageBox::information(this, "错误", "请输入用户名和密码");
    }
}

bool LoginDialog::login(const string &username, const string &password, bool messagebox) {
    map<string, string> data;
    string res;
    data["username"] = username;
    data["password"] = password;
    auto req = std::make_shared<HTTPRequest>(m_ip.data(), m_port);
    req->init();
    bool flag = req->post("/login", data, res);
    req->close_socket();
    if (flag) {
        try {
            Document document;
            document.Parse(res.c_str());
            assert(document.IsObject());
            assert(document.HasMember("status"));
            assert(document.HasMember("code"));
            if (document["status"].GetInt() == 200) {
                if (document["code"].GetInt() == 1) {
                    extern Config g_config;
                    std::ofstream out(g_config.login_cache_path);
                    if (out.is_open()) {
                        out << username << std::endl;
                        out << password << std::endl;
                        out.close();
                        g_config.token = document["token"].GetString();
                        return true;
                    }
                }
                if (messagebox) {
                    if (document.HasMember("msg")) {
                        QMessageBox::information(this, "错误", document["msg"].GetString());

                    }
                    else {
                        QMessageBox::information(this, "错误", "登录仅本次有效");
                    }
                }
            }
            else {
                QMessageBox::information(this, "错误", "网络错误");
            }
        }
        catch (std::exception &e) {
            LOG_ERROR("exception:%s", e.what())
        }
    }
    else {
        if (messagebox)
            QMessageBox::information(this, "错误", "服务器或网络错误，请重试");
    }
    return false;
}

void LoginDialog::slot_register() {
    string username = ui->rUserName->text().toStdString();
    string password1 = ui->rPassword1->text().toStdString();
    string password2 = ui->rPassword2->text().toStdString();
    if (!username.empty() && !password1.empty() && !password2.empty()) {
        if (password1 == password2) {
            map<string, string> data;
            string res;
            data["username"] = username;
            data["password1"] = password1;
            data["password2"] = password2;
            auto req = std::make_shared<HTTPRequest>(m_ip, m_port);
            req->init();
            bool flag = req->post("/register", data, res);
            req->close_socket();
            if (flag) {
                try {
                    Document document;
                    document.Parse(res.c_str());
                    assert(document.IsObject());
                    assert(document.HasMember("status"));
                    assert(document.HasMember("code"));
                    if (document["status"].GetInt() == 200 && document["code"].GetInt() == 1) {
                        QMessageBox::information(this, "成功", "注册成功");
                        ui->TabWidget->setCurrentIndex(0);
                        this->slot_reset();
                    }
                    else {
                        QMessageBox::information(this, "错误", document["msg"].GetString());
                    }
                }
                catch (std::exception &e) {

                }
            }
            else {
                QMessageBox::information(this, "错误", "网络请求错误，请重试");
            }
        }
        else {
            QMessageBox::information(this, "错误", "两次输入的密码不一致");
        }
    }
    else {
        QMessageBox::information(this, "错误", "请填写完整信息");
    }
}

void LoginDialog::slot_reset() {
    ui->rUserName->clear();
    ui->rPassword1->clear();
    ui->rPassword2->clear();
}
