//
// Created by lisongqian on 2022/5/9.
//

#ifndef NETDISK_MAINWINDOW_H
#define NETDISK_MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QProxyStyle>
#include <QStylePainter>
#include "ui_mainWindow.h"
#include "LoginDialog.h"
#include "components/ClickLabel.h"

class MainWindow : public QMainWindow {
Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void addConnection(LoginDialog *dialog) const;
    void showFileNavigation(bool isShow = true);
    void addNavigation(std::string_view name);
    ~MainWindow() override = default;
public slots:
    void show_myself();
    void changeTab(int currentRow );
    void navigationClick(int index);
private:
    std::shared_ptr<Ui::MainWindow> ui;
    std::vector<std::shared_ptr<ClickLabel>> m_navigation;
};


#endif //NETDISK_MAINWINDOW_H
