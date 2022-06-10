//
// Created by lisongqian on 2022/5/9.
//

#ifndef NETDISK_MAINWINDOW_H
#define NETDISK_MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QProxyStyle>
#include <QStylePainter>
#include <QStandardItemModel>
#include <QFileInfo>
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

    void changeTab(int currentRow);

    void navigationClick(int index);

    void updateFileList();

    static void uploadFile(std::vector<std::shared_ptr<QFileInfo>> &files);

    void downloadFile();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;

    void dropEvent(QDropEvent *event);

private:
    std::shared_ptr<Ui::MainWindow> ui;
    std::vector<std::shared_ptr<ClickLabel>> m_navigation;
    std::shared_ptr<QStandardItemModel> m_file_list_model;
    std::shared_ptr<QStandardItemModel> m_file_share_model;     // TODO 服务器记录
    std::shared_ptr<QStandardItemModel> m_file_download_model;  // TODO 本地存储记录
    std::shared_ptr<QStandardItemModel> m_file_upload_model;    // TODO 本地存储记录
};


#endif //NETDISK_MAINWINDOW_H
