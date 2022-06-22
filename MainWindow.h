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

    void addNavigation(std::string_view name,int id);
    void rename(int type, int id, std::string name);


    ~MainWindow() override = default;

public slots:

    void slot_show_myself();

    void changeTab(int currentRow);

    void navigationClick(int index);

    void slot_updateFileList();

    void slot_uploadFile(std::vector<std::shared_ptr<QFileInfo>> &files);

    void slot_downloadFile();

    void slot_customContextMenu(const QPoint &pos);

    void slot_mkdir(std::string dir_name);

    void slot_rename();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;

    void dropEvent(QDropEvent *event) override;

private:
    std::shared_ptr<Ui::MainWindow> ui;
    std::vector<std::shared_ptr<ClickLabel>> m_navigation;
    std::shared_ptr<QStandardItemModel> m_file_list_model;
    std::shared_ptr<QStandardItemModel> m_file_share_model;     // TODO 服务器记录
    std::shared_ptr<QStandardItemModel> m_file_download_model;  // TODO 本地存储记录
    std::shared_ptr<QStandardItemModel> m_file_upload_model;    // TODO 本地存储记录
    int m_current_dir;
};


#endif //NETDISK_MAINWINDOW_H
