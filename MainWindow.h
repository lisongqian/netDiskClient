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
#include <fstream>
#include "ui_mainWindow.h"
#include "LoginDialog.h"
#include "components/ClickLabel.h"

enum FileListHeaderColumn {
    FILE_NAME = 0,
    MODIFY_TIME,
    FILE_TYPE_STR,
    FILE_SIZE,
    FILE_ID,
    FILE_HASH,
    FILE_TYPE_VALUE
};

enum class LocalListHeaderColumn {
    FILE_NAME = 0,
    FILE_SIZE,
    FILE_TIME,
    FILE_PATH,
};

class MainWindow : public QMainWindow {
Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    void addConnection(LoginDialog *dialog) const;

    void showFileNavigation(bool isShow = true);

    void addNavigation(std::string_view name, int id);

    void rename(int type, int id, std::string name);

    ~MainWindow() override;

public slots:

    void slot_show_myself();

    void changeTab(int currentRow);

    void navigationClick(int index);

    /**
     * 更新网盘文件列表
     */
    void slot_updateFileList();

    /**
     * 更新分享记录
     */
    void slot_updateShareRecords();

    /**
     * 更新上传记录
     */
    void slot_updateUploadRecords();

    /**
     * 更新下载记录
     */
    void slot_updateDownloadRecords();

    void slot_uploadFile(std::vector<std::shared_ptr<QFileInfo>> &files);

    void slot_downloadFile();

    void slot_customContextMenu(const QPoint &pos);

    void slot_mkdir(std::string dir_name);

    void slot_rename();

    void slot_deleteFile();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;

    void dropEvent(QDropEvent *event) override;

private:
    std::shared_ptr<Ui::MainWindow> ui;
    std::vector<std::shared_ptr<ClickLabel>> m_navigation;
    std::shared_ptr<QStandardItemModel> m_file_list_model;
    std::shared_ptr<QStandardItemModel> m_file_share_model;     // TODO 服务器记录
    std::shared_ptr<QStandardItemModel> m_file_download_model;  //  本地存储记录
    std::shared_ptr<QStandardItemModel> m_file_upload_model;    //  本地存储记录
    int m_current_dir;
    std::fstream m_local_download_file;
    std::fstream m_local_upload_file;
    string m_download_score_file_path;
    string m_upload_score_file_path;
};


#endif //NETDISK_MAINWINDOW_H
