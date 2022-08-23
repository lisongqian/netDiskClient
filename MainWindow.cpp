//
// Created by lisongqian on 2022/5/9.
//

#include <QFile>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QMenu>
#include <QFileDialog>
#include <utility>
#include <direct.h>
#include <thread>
#include <QTextCodec>
#include <QInputDialog>
#include "MainWindow.h"
#include "request/HTTPRequest.h"    // winsock2.h 要在windows.h 前 locker.h中引用了windows.h
#include "log/log.h"
#include "config.h"
#include "rapidjson/document.h"

using namespace rapidjson;
extern Config g_config;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(std::make_shared<Ui::MainWindow>()),
                                          m_current_dir(0) {
    ui->setupUi(this);
    if (QFile qss(":/main.qss");qss.open(QFile::ReadOnly)) {
        this->setStyleSheet(qss.readAll());
        qss.close();
    }
    /*主窗口设置*/
    ui->statusbar->hide();
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setFixedSize(this->width(), this->height());
    setAcceptDrops(true);

    /*左侧栏设置*/
    ui->leftWidget->setAttribute(Qt::WidgetAttribute::WA_StyledBackground);
    int icon_size = 25;
    ui->listWidget->setIconSize(QSize(icon_size, icon_size));
    ui->listWidget->setCurrentRow(0);
    ui->tabWidget->setCurrentIndex(0);
    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &MainWindow::changeTab);

    /*文件导航栏*/
    auto label = std::make_shared<ClickLabel>(tr("文件"), 0, ui->navigation);
    label->adjustSize();
    label->setGeometry(QRect(0, 0, label->width() + 8, ui->navigation->height()));
    connect(label.get(), &ClickLabel::clicked, this, &MainWindow::navigationClick);
    label->setCursor(QCursor(Qt::PointingHandCursor));
    label->setProperty("level", "last");
    label->style()->polish(label.get());
    m_navigation.push_back(std::move(label));
    showFileNavigation();

    /*右键菜单设置*/
    ui->fileTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->fileTableView, &QAbstractItemView::customContextMenuRequested, this,
            &MainWindow::slot_customContextMenu);


    /*设置表格属性*/
    m_file_list_model = std::make_shared<QStandardItemModel>();
    QStringList fileListHeaders;
    fileListHeaders << "文件名" << "修改时间" << "类型" << "文件大小" << "id" << "哈希值" << "类型值";
    m_file_list_model->setColumnCount(static_cast<int>(fileListHeaders.size()));
    m_file_list_model->setHorizontalHeaderLabels(fileListHeaders);
    ui->fileTableView->setModel(m_file_list_model.get());
    ui->fileTableView->verticalHeader()->hide();
    ui->fileTableView->setEditTriggers(QTableView::NoEditTriggers);
    ui->fileTableView->setShowGrid(false);
    ui->fileTableView->setColumnWidth(FileListHeaderColumn::FILE_NAME, 220);
    ui->fileTableView->setColumnWidth(FileListHeaderColumn::MODIFY_TIME, 150);
    ui->fileTableView->setColumnWidth(FileListHeaderColumn::FILE_TYPE_STR, 50);
    ui->fileTableView->setColumnWidth(FileListHeaderColumn::FILE_SIZE, 80);
    ui->fileTableView->setColumnHidden(FileListHeaderColumn::FILE_ID, true);
    ui->fileTableView->setColumnHidden(FileListHeaderColumn::FILE_HASH, true);
    ui->fileTableView->setColumnHidden(FileListHeaderColumn::FILE_TYPE_VALUE, true);
    ui->fileTableView->setSelectionBehavior(QTableView::SelectRows);
    ui->fileTableView->setSelectionMode(QTableView::SingleSelection);
//    ui->fileTableView->setAlternatingRowColors(true);

    m_file_download_model = std::make_shared<QStandardItemModel>();
    QStringList download_headers;
    // 文件名，文件大小，时间，本地路径
    download_headers << "文件名" << "文件大小" << "时间" << "本地路径";
    m_file_download_model->setColumnCount(static_cast<int>(download_headers.size()));
    m_file_download_model->setHorizontalHeaderLabels(download_headers);
    ui->downloadTableView->setModel(m_file_download_model.get());
//    ui->downloadTableView->verticalHeader()->hide();
    ui->downloadTableView->setEditTriggers(QTableView::NoEditTriggers);
    ui->downloadTableView->setShowGrid(false);
    ui->downloadTableView->setSelectionBehavior(QTableView::SelectRows);
    ui->downloadTableView->setSelectionMode(QTableView::SingleSelection);
    ui->downloadTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
//    ui->downloadTableView->setColumnWidth(static_cast<int>(LocalListHeaderColumn::FILE_NAME), 100);
//    ui->downloadTableView->setColumnWidth(static_cast<int>(LocalListHeaderColumn::FILE_SIZE), 60);
//    ui->downloadTableView->setColumnWidth(static_cast<int>(LocalListHeaderColumn::FILE_TIME), 140);
//        ui->downloadTableView->setColumnWidth(static_cast<int>(LocalListHeaderColumn::FILE_PATH), 220);
    ui->downloadTableView->setColumnHidden(static_cast<int>(LocalListHeaderColumn::FILE_PATH), true);

    // 文件名，文件大小，网盘路径，时间
    m_file_upload_model = std::make_shared<QStandardItemModel>();
    QStringList upload_headers;
    upload_headers << "文件名" << "文件大小" << "时间" << "网盘路径";
    m_file_upload_model->setColumnCount(static_cast<int>(upload_headers.size()));
    m_file_upload_model->setHorizontalHeaderLabels(upload_headers);
    ui->uploadTableView->setModel(m_file_upload_model.get());
    ui->uploadTableView->setEditTriggers(QTableView::NoEditTriggers);
    ui->uploadTableView->setShowGrid(false);
    ui->uploadTableView->setSelectionBehavior(QTableView::SelectRows);
    ui->uploadTableView->setSelectionMode(QTableView::SingleSelection);
    ui->uploadTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);




    /*表格双击功能*/
    connect(ui->fileTableView, &QAbstractItemView::doubleClicked, this, [=](const QModelIndex &index) {
        m_queue_lock.lock();
        QVariant v = m_file_list_model->data(m_file_list_model->index(index.row(), 6));
        if (v.toString().toStdString() == "0") {
            m_current_dir = m_file_list_model->data(m_file_list_model->index(index.row(), 4)).toInt();
            addNavigation(m_file_list_model->data(m_file_list_model->index(index.row(), 0)).toString().toStdString(),
                          m_current_dir);
            m_queue_lock.unlock();
            slot_updateFileList();
        }
        else {
            m_queue_lock.unlock();
        }
    });
    /*本地用户记录*/
    string &&tmp_dir = "tmp/" + g_config.token;
    if (0 != access(tmp_dir.c_str(), 0)) {
        mkdir(tmp_dir.c_str());
    }
    m_download_score_file_path = tmp_dir + "/download_scores.txt";
    m_upload_score_file_path = tmp_dir + "/upload_scores.txt";
    slot_updateDownloadRecords();
    slot_updateUploadRecords();
    m_local_download_file.open(m_download_score_file_path, std::ios::in | std::ios::app);
    m_local_upload_file.open(m_upload_score_file_path, std::ios::in | std::ios::app);

    connect(this, &MainWindow::sig_information, this, &MainWindow::slot_message);
}

/**
 * 显示窗口
 */
void MainWindow::slot_show_myself() {
    slot_updateFileList();
    m_navigation[0]->m_dir_id = m_current_dir;  // 初始化根目录id
    this->show();
}

/**
 * 连接信号，用于触发内部函数
 * @param dialog
 */
void MainWindow::addConnection(LoginDialog *dialog) const {
    connect(dialog, &LoginDialog::open_main_window, this, &MainWindow::slot_show_myself);
}

/**
 * 切换标签页
 * @param currentRow
 */
void MainWindow::changeTab(int currentRow) {
    if (currentRow >= 0) {
//        LOG_DEBUG("changeTab:%d", currentRow);
        ui->tabWidget->setCurrentIndex(currentRow);
        switch (currentRow) {
            case 0: {
//                updateFileList();
                break;
            }
            case 1: {
                break;
            }
            case 2: {
                break;
            }
            case 3: {
                break;
            }
            default:
                break;
        }
    }
}

/**
 * 点击导航栏
 * @param index
 */
void MainWindow::navigationClick(int index) {
//    LOG_DEBUG("click index:%d", index)
    /**
     * index 为负数为 ">"，点击无效果
     */
    if (index < 0) return;
    while (m_navigation.size() > index * 2 + 1) {
        m_navigation.pop_back();
    }
    m_navigation[index * 2]->setProperty("level", "last");
    m_navigation[index * 2]->style()->polish(m_navigation[index * 2].get());
    m_current_dir = m_navigation[m_navigation.size() - 1]->m_dir_id;
    slot_updateFileList();
}

/**
 * 显示文件导航栏
 */
void MainWindow::showFileNavigation(bool isShow) {
    if (isShow) {
        for (const auto &item: m_navigation) {
            item->show();
        }
    }
    else {
        for (const auto &item: m_navigation) {
            item->hide();
        }
    }
}

void MainWindow::addNavigation(std::string_view name, int id) {
//    LOG_DEBUG("addNavigation:%s", name.data());
    QPoint point = m_navigation[m_navigation.size() - 1]->pos();
    auto tag = std::make_shared<ClickLabel>(-1, ui->navigation);
    QIcon icon = QIcon(":/images/caret-right.svg");
    QPixmap m_pic = icon.pixmap(icon.actualSize(QSize(10, 10)));//size自行调整
    tag->setPixmap(m_pic);
    tag->adjustSize();
    tag->setGeometry(QRect(point.x() + m_navigation[m_navigation.size() - 1]->width(), 0, tag->width(),
                           ui->navigation->height()));
    tag->show();
    auto label = std::make_shared<ClickLabel>(tr(name.data()), (m_navigation.size() + 1) / 2, ui->navigation);
    label->m_dir_id = id;
    label->adjustSize();
    label->setGeometry(
            QRect(point.x() + m_navigation[m_navigation.size() - 1]->width() + tag->width(), 0, label->width() + 8,
                  ui->navigation->height()));
    connect(label.get(), &ClickLabel::clicked, this, &MainWindow::navigationClick);
    label->setCursor(QCursor(Qt::PointingHandCursor));
    label->show();

    m_navigation[m_navigation.size() - 1]->setProperty("level", "normal");
    m_navigation[m_navigation.size() - 1]->style()->polish(m_navigation[m_navigation.size() - 1].get());
    label->setProperty("level", "last");
    label->style()->polish(label.get());
    m_navigation.push_back(std::move(tag));
    m_navigation.push_back(std::move(label));
}

void MainWindow::slot_updateFileList() {
//    std::thread thread([=]() {
    string res;
    map<string, string> headers;
    headers["Token"] = g_config.token;
    map<string, string> data;
    data["dir"] = std::to_string(m_current_dir);
    auto req = std::make_shared<HTTPRequest>(g_config.ip.data(), g_config.port);
    req->init();
    bool flag = req->post("/filelist", data, headers, res);
    if (flag) {
        try {
//                LOG_DEBUG("res:%s", res.c_str());
            Document document;
            document.Parse(res.c_str());
            if (document.IsObject() && document.HasMember("status") && document.HasMember("code")) {
                if (document["status"].GetInt() == 200 && document["code"].GetInt() == 1) {
                    Value &list = document["data"];
                    if (list.IsArray()) {
                        m_queue_lock.lock();
                        m_file_list_model->removeRows(0,
                                                      m_file_list_model->rowCount());// TODO Abort Issue and I dont know why
                        m_file_list_model->setRowCount(static_cast<int>(list.Size()));
                        for (int i = 0; i < list.Size(); ++i) {
                            Value &item = list[i];
                            if (item.IsObject()) {
                                m_file_list_model->setItem(i, FileListHeaderColumn::FILE_NAME,
                                                           new QStandardItem(item["name"].GetString()));
                                m_file_list_model->item(i,
                                                        static_cast<int>(FileListHeaderColumn::FILE_NAME))->setTextAlignment(
                                        Qt::AlignCenter);
                                m_file_list_model->setItem(i, FileListHeaderColumn::MODIFY_TIME,
                                                           new QStandardItem(item["time"].GetString()));
                                m_file_list_model->item(i,
                                                        static_cast<int>(FileListHeaderColumn::MODIFY_TIME))->setTextAlignment(
                                        Qt::AlignCenter);
                                if (strcmp(item["type"].GetString(), "0") == 0) {
                                    m_file_list_model->setItem(i, FileListHeaderColumn::FILE_TYPE_STR,
                                                               new QStandardItem("目录"));
                                    m_file_list_model->setItem(i, FileListHeaderColumn::FILE_SIZE,
                                                               new QStandardItem("-"));
                                }
                                else {
                                    m_file_list_model->setItem(i, FileListHeaderColumn::FILE_TYPE_STR,
                                                               new QStandardItem("文件"));
                                    m_file_list_model->setItem(i, FileListHeaderColumn::FILE_SIZE,
                                                               new QStandardItem(
                                                                       file_size_display(
                                                                               strtod(item["size"].GetString(),
                                                                                      nullptr)).data()));
                                }
                                m_file_list_model->item(i,
                                                        static_cast<int>(FileListHeaderColumn::FILE_TYPE_STR))->setTextAlignment(
                                        Qt::AlignCenter);
                                m_file_list_model->item(i,
                                                        static_cast<int>(FileListHeaderColumn::FILE_SIZE))->setTextAlignment(
                                        Qt::AlignCenter);
                                m_file_list_model->setItem(i, FileListHeaderColumn::FILE_ID,
                                                           new QStandardItem(item["id"].GetString()));
                                m_file_list_model->item(i,
                                                        static_cast<int>(FileListHeaderColumn::FILE_ID))->setTextAlignment(
                                        Qt::AlignCenter);
                                m_file_list_model->setItem(i, FileListHeaderColumn::FILE_HASH,
                                                           new QStandardItem(item["hash"].GetString()));
                                m_file_list_model->item(i,
                                                        static_cast<int>(FileListHeaderColumn::FILE_HASH))->setTextAlignment(
                                        Qt::AlignCenter);
                                m_file_list_model->setItem(i, FileListHeaderColumn::FILE_TYPE_VALUE,
                                                           new QStandardItem(item["type"].GetString()));
                                m_file_list_model->item(i,
                                                        static_cast<int>(FileListHeaderColumn::FILE_TYPE_VALUE))->setTextAlignment(
                                        Qt::AlignCenter);
                                if (!i) {// i==0
                                    m_current_dir = item["parent"].GetInt();
                                }
                            }
                        }
                        m_queue_lock.unlock();
                    }
                }
            }
            else {
                emit sig_information(this, "错误", "请重试");
            }
        }
        catch (std::exception &e) {
        }
    }
    req->close_socket();
//    });
//    thread.detach();
}

void MainWindow::slot_uploadFile(std::vector<std::shared_ptr<QFileInfo>> &files) {
    std::thread thread([=]() {
        string res;
        map<string, string> headers;
        headers["Token"] = g_config.token;
        headers["Content-Type"] = "multipart/form-data; boundary=--boundary_yitiaohunxiaozifuchuanchuan";
        auto req = std::make_shared<HTTPRequest>(g_config.ip.data(), g_config.port);
        req->init();
        bool flag = req->sendFile("/upload", files, m_current_dir, headers, res);
        if (flag) {
//            LOG_DEBUG("%s", res.c_str());
            Document document;
            document.Parse(res.c_str());
            if (document.IsObject() && document.HasMember("status") && document.HasMember("code")) {
                if (document["status"].GetInt() == 200 && document["code"].GetInt() == 1) {
                    emit sig_information(this, "信息", "上传成功!");
                    if (m_local_upload_file.is_open()) {
                        int current = m_file_upload_model->rowCount();
                        for (int i = 0; i < files.size(); i++) {

                            string file_name = files[i]->fileName().toStdString();
                            string file_size = file_size_display(static_cast<double>(files[i]->size()));
                            string file_path = m_navigation[m_navigation.size() - 1]->text().toStdString();
                            time_t t = time(nullptr);
                            char now_time[32] = {'\0'};
                            strftime(now_time, sizeof(now_time), "%Y-%m-%d_%H:%M", localtime(&t));
                            string time_str = now_time;
                            m_local_upload_file << file_name << " ";
                            m_local_upload_file << file_size << " ";
                            m_local_upload_file << file_path << " ";
                            m_local_upload_file << time_str << std::endl;
                            m_file_upload_model->setItem(current + i,
                                                         static_cast<int>(LocalListHeaderColumn::FILE_NAME),
                                                         new QStandardItem(file_name.c_str()));
                            m_file_upload_model->item(current + i,
                                                      static_cast<int>(LocalListHeaderColumn::FILE_NAME))->setTextAlignment(
                                    Qt::AlignCenter);
                            m_file_upload_model->setItem(current + i,
                                                         static_cast<int>(LocalListHeaderColumn::FILE_SIZE),
                                                         new QStandardItem(file_size.c_str()));
                            m_file_upload_model->item(current + i,
                                                      static_cast<int>(LocalListHeaderColumn::FILE_SIZE))->setTextAlignment(
                                    Qt::AlignCenter);

                            m_file_upload_model->setItem(current + i,
                                                         static_cast<int>(LocalListHeaderColumn::FILE_PATH),
                                                         new QStandardItem(file_path.c_str()));
                            m_file_upload_model->item(current + i,
                                                      static_cast<int>(LocalListHeaderColumn::FILE_PATH))->setTextAlignment(
                                    Qt::AlignCenter);

                            time_str.replace(time_str.find('_'), 1, " ");
                            m_file_upload_model->setItem(current + i,
                                                         static_cast<int>(LocalListHeaderColumn::FILE_TIME),
                                                         new QStandardItem(time_str.c_str()));
                            m_file_upload_model->item(current + i,
                                                      static_cast<int>(LocalListHeaderColumn::FILE_TIME))->setTextAlignment(
                                    Qt::AlignCenter);
                        }
                    }
                    slot_updateFileList();
                }
                else {
                    emit sig_information(this, "信息", document["msg"].GetString());

                }
            }
            else {
                emit sig_information(this, "错误", "请重试");
            }

        }
        req->close_socket();
    });
    thread.detach();
}

void MainWindow::slot_downloadFile() {
    std::thread thread([=]() {
        map<string, string> headers;
        headers["Token"] = g_config.token;
        map<string, string> data;
        m_queue_lock.lock();
//    data["filename"] = ;
//    data["id"] = m_file_list_model->index(ui->fileTableView->currentIndex().row(), 4).data().toString().toStdString();
        data["hash"] = m_file_list_model->index(ui->fileTableView->currentIndex().row(),
                                                FileListHeaderColumn::FILE_HASH).data().toString().toStdString();
        // windows 本地编码为gbk
        QTextCodec *gbk = QTextCodec::codecForName("gbk");
        string filename = gbk->fromUnicode(m_file_list_model->index(ui->fileTableView->currentIndex().row(),
                                                                    FileListHeaderColumn::FILE_NAME).data().toString().toStdString().c_str()).data();
        m_queue_lock.unlock();
        auto req = std::make_shared<HTTPRequest>(g_config.ip.data(), g_config.port);
        req->init();
        bool flag = req->downloadFile("/download", data, headers, filename);
        if (flag) {
            emit sig_information(this, "成功", "下载完成");
            if (m_local_download_file.is_open()) {
                m_queue_lock.lock();
                string file_name = m_file_list_model->index(ui->fileTableView->currentIndex().row(),
                                                            FileListHeaderColumn::FILE_NAME).data().toString().toStdString();
                string file_size = m_file_list_model->index(ui->fileTableView->currentIndex().row(),
                                                            FileListHeaderColumn::FILE_SIZE).data().toString().toStdString();
                m_queue_lock.unlock();
                string file_path = g_config.download_path + file_name;
                time_t t = time(nullptr);
                char now_time[32] = {'\0'};
                strftime(now_time, sizeof(now_time), "%Y-%m-%d_%H:%M", localtime(&t));
                string time_str = now_time;
                m_local_download_file << file_name << " ";
                m_local_download_file << file_size << " ";
                m_local_download_file << file_path << " ";
                m_local_download_file << time_str << std::endl;
                int current = m_file_download_model->rowCount();
                m_file_download_model->setItem(current, static_cast<int>(LocalListHeaderColumn::FILE_NAME),
                                               new QStandardItem(file_name.c_str()));
                m_file_download_model->item(current,
                                            static_cast<int>(LocalListHeaderColumn::FILE_NAME))->setTextAlignment(
                        Qt::AlignCenter);
                m_file_download_model->setItem(current, static_cast<int>(LocalListHeaderColumn::FILE_SIZE),
                                               new QStandardItem(file_size.c_str()));
                m_file_download_model->item(current,
                                            static_cast<int>(LocalListHeaderColumn::FILE_SIZE))->setTextAlignment(
                        Qt::AlignCenter);
                m_file_download_model->setItem(current, static_cast<int>(LocalListHeaderColumn::FILE_PATH),
                                               new QStandardItem(file_path.c_str()));
                m_file_download_model->item(current,
                                            static_cast<int>(LocalListHeaderColumn::FILE_PATH))->setTextAlignment(
                        Qt::AlignCenter);

                time_str.replace(time_str.find('_'), 1, " ");
                m_file_download_model->setItem(current, static_cast<int>(LocalListHeaderColumn::FILE_TIME),
                                               new QStandardItem(time_str.c_str()));
                m_file_download_model->item(current,
                                            static_cast<int>(LocalListHeaderColumn::FILE_TIME))->setTextAlignment(
                        Qt::AlignCenter);
            }
        }
        else {
            emit sig_information(this, "错误", "下载失败");
        }
        req->close_socket();
    });
    thread.detach();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
//    if (event->mimeData()->hasUrls())
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event) {
    QList<QUrl> urls = event->mimeData()->urls();
    std::vector<std::shared_ptr<QFileInfo>> files;
    QString suffixs = "sh exe bat";

    for (const QUrl &url: urls) {
        auto file = std::make_shared<QFileInfo>(url.toLocalFile());    //toLocalFile可以获取文件路径，而非QUrl的file://开头的路径
        if (file->isFile() && !suffixs.contains(file->suffix())) //过滤掉目录和不支持的后缀，如果要支持目录，则要自己遍历每一个目录。
            files.push_back(file);
    }
    slot_uploadFile(files);
}

void MainWindow::slot_customContextMenu(const QPoint &pos) {
    switch (ui->tabWidget->currentIndex()) {
        case 0: {
            auto row = ui->fileTableView->currentIndex().row(); // 选中的行
            auto type_index = m_file_list_model->index(row, FileListHeaderColumn::FILE_TYPE_VALUE);
            // 参数 pos 是鼠标按下的位置, 但是不能直接使用, 这个坐标不是屏幕坐标, 是当前窗口的坐标
            // 如果要使用这个坐标需要将其转换为屏幕坐标
            QMenu menu;
            QAction *act = menu.addAction("刷新");
            connect(act, &QAction::triggered, this, &MainWindow::slot_updateFileList);
            menu.addSeparator();
            act = menu.addAction("下载文件");
            if (m_file_list_model->data(type_index).toInt() == 0) {
                act->setEnabled(false);
            }
            connect(act, &QAction::triggered, this, &MainWindow::slot_downloadFile);
            act = menu.addAction("上传文件");
            connect(act, &QAction::triggered, this, [=]() {
                QString fileName = QFileDialog::getOpenFileName(
                        this,
                        tr("打开文件"),
                        "D:/Downloads",
                        tr("所有文件(*.*)"));
                if (fileName.isEmpty()) {
//                    QMessageBox::warning(this, "Warning!", "Failed to open the video!");
                    return;
                }
                LOG_INFO("%s", fileName.toStdString().c_str())
                std::vector<std::shared_ptr<QFileInfo>> files;
                auto file = std::make_shared<QFileInfo>(
                        fileName.toStdString().c_str());    //toLocalFile可以获取文件路径，而非QUrl的file://开头的路径
                files.push_back(file);
                slot_uploadFile(files);
            });
            act = menu.addAction("新建文件夹");
            connect(act, &QAction::triggered, this, [=]() {
                slot_mkdir("新建文件夹");
            });
            menu.addSeparator();
            act = menu.addAction("分享文件");
            if (m_file_list_model->data(type_index).toInt() == 0) {
                act->setEnabled(false);
            }
            connect(act, &QAction::triggered, this, &MainWindow::slot_share_file);
            menu.addSeparator();
            act = menu.addAction("重命名");
            connect(act, &QAction::triggered, this, &MainWindow::slot_rename);
            menu.addSeparator();
            if (m_file_list_model->data(type_index).toInt() == 0) {
                act = menu.addAction("删除文件夹");
            }
            else {
                act = menu.addAction("删除文件");
            }
            connect(act, &QAction::triggered, this, &MainWindow::slot_deleteFile);
            // menu.exec(QCursor::pos());
            // 将窗口坐标转换为屏幕坐标
            QPoint newpt = ui->fileTableView->mapToGlobal(pos);
            menu.exec(newpt);

        }
        default:
            break;
    }
}

void MainWindow::slot_mkdir(std::string dir_name) {
    string res;
    auto req = std::make_shared<HTTPRequest>(g_config.ip.data(), g_config.port);
    req->init();
    map<string, string> headers;
    headers["token"] = g_config.token;
    map<string, string> data;
    data["parent"] = std::to_string(m_current_dir);
    data["name"] = std::move(dir_name);
    int flag = req->post("/mkdir", data, headers, res);
    if (flag) {
        Document document;
        document.Parse(res.c_str());
        if (document.IsObject() && document.HasMember("status") && document.HasMember("code")) {
            if (document["status"].GetInt() == 200 && document["code"].GetInt() == 1) {
                slot_updateFileList();
//                Sleep(20);
                slot_rename();
            }
            else {
                emit sig_information(this, "错误", document["msg"].GetString());
            }
        }
        else {
            emit sig_information(this, "错误", "请重试");
        }
    }
    req->close_socket();
}

void MainWindow::rename(int type, int id, std::string name) {

    auto req = std::make_shared<HTTPRequest>(g_config.ip.data(), g_config.port);
    req->init();
    string res;
    map<string, string> data;
    data["type"] = std::to_string(type);
    data["id"] = std::to_string(id);
    data["name"] = std::move(name);
    map<string, string> headers;
    headers["token"] = g_config.token;
    int flag = req->post("/rename", data, headers, res);
    if (flag) {
        Document document;
        document.Parse(res.c_str());
        if (document.IsObject() && document.HasMember("status") && document.HasMember("code")) {
            if (document["status"].GetInt() == 200 && document["code"].GetInt() == 1) {
                LOG_INFO("rename success")
            }
            else {
                emit sig_information(this, "错误", document["msg"].GetString());
                slot_updateFileList();
            }
        }
        else {
            emit sig_information(this, "错误", "修改失败");
            slot_updateFileList();
        }
    }
    req->close_socket();
}

/**
 * 重命名用
 * @param topLeft
 * @param bottomRight
 */
void MainWindow::slot_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) {
    LOG_DEBUG("slot_dataChanged触发,index:%d,%d", topLeft.row(), topLeft.column())
    if (topLeft == bottomRight) {
        if (topLeft.column() == 0) {
            if (ui->fileTableView->isPersistentEditorOpen(topLeft)) {
                ui->fileTableView->closePersistentEditor(topLeft);
                m_queue_lock.lock();
                int type = m_file_list_model->data(
                        m_file_list_model->index(topLeft.row(), FileListHeaderColumn::FILE_TYPE_VALUE)).toInt();
                int id = m_file_list_model->data(
                        m_file_list_model->index(topLeft.row(), FileListHeaderColumn::FILE_ID)).toInt();
                string name = m_file_list_model->data(topLeft).toString().toStdString();
                m_queue_lock.unlock();
                rename(type, id, name);
                disconnect(m_file_list_model.get(), &QAbstractItemModel::dataChanged, this,
                           &MainWindow::slot_dataChanged);
            }
        }
    }
}

void MainWindow::slot_rename() {
    /*重命名功能*/
    connect(m_file_list_model.get(), &QAbstractItemModel::dataChanged, this, &MainWindow::slot_dataChanged);
    m_queue_lock.lock();
    auto index = m_file_list_model->index(ui->fileTableView->currentIndex().row(), 0);
    ui->fileTableView->openPersistentEditor(index);
    ui->fileTableView->setFocus();
    QWidget *editWidget = ui->fileTableView->indexWidget(index);
    m_queue_lock.unlock();
    if (editWidget != nullptr) {
        editWidget->setFocus();
        disconnect(editWidget, SIGNAL(editingFinished()), editWidget, SLOT(close()));
        connect(editWidget, SIGNAL(editingFinished()), editWidget, SLOT(close()));
    }
}

void MainWindow::slot_deleteFile() {
    std::thread thread([=]() {
        map<string, string> headers;
        headers["Token"] = g_config.token;
        int row = ui->fileTableView->currentIndex().row();
//        LOG_INFO("row_num:%d", row)
        m_queue_lock.lock();
        QModelIndex id_index = m_file_list_model->index(row, FileListHeaderColumn::FILE_ID);
        QModelIndex type_index = m_file_list_model->index(row, FileListHeaderColumn::FILE_TYPE_VALUE);
        map<string, string> data;
        data["id"] = m_file_list_model->data(id_index).toString().toStdString();
        data["type"] = m_file_list_model->data(type_index).toString().toStdString();
        LOG_INFO("filename:%s", m_file_list_model->data(
                m_file_list_model->index(row, FileListHeaderColumn::FILE_NAME)).toString().toStdString().c_str())
        m_queue_lock.unlock();
        string res;
        auto req = std::make_shared<HTTPRequest>(g_config.ip.data(), g_config.port);
        req->init();
        bool flag = req->post("/deletefile", data, headers, res);
        if (flag) {
            Document document;
            document.Parse(res.c_str());
            if (document.IsObject() && document.HasMember("status") && document.HasMember("code")) {
                if (document["status"].GetInt() == 200 && document["code"].GetInt() == 1) {
                    slot_updateFileList();
                }
                else {
                    emit sig_information(this, "错误", document["msg"].GetString());
                }
            }
            else {
                emit sig_information(this, "错误", "请重试");
            }
        }
        req->close_socket();
    });
    thread.detach();
}

void MainWindow::slot_updateShareRecords() {

}

void MainWindow::slot_updateUploadRecords() {
    std::fstream fout(m_upload_score_file_path.c_str(), std::ios::in);
    if (fout.is_open()) {
        int records_num = 0, model_num = m_file_download_model->rowCount();
        if (records_num < model_num) {
            m_file_upload_model->removeRows(records_num - 1, m_file_upload_model->rowCount() - records_num);
        }
        string file_name, file_size, file_path, file_time;
        for (int i = 0; fout >> file_name >> file_size >> file_path >> file_time; ++i) {
            file_time.replace(file_time.find('_'), 1, " ");
            m_file_upload_model->setItem(i, static_cast<int>(LocalListHeaderColumn::FILE_NAME),
                                         new QStandardItem(file_name.c_str()));
            m_file_upload_model->item(i, static_cast<int>(LocalListHeaderColumn::FILE_NAME))->setTextAlignment(
                    Qt::AlignCenter);
            m_file_upload_model->setItem(i, static_cast<int>(LocalListHeaderColumn::FILE_SIZE),
                                         new QStandardItem(file_size.c_str()));
            m_file_upload_model->item(i, static_cast<int>(LocalListHeaderColumn::FILE_SIZE))->setTextAlignment(
                    Qt::AlignCenter);
            m_file_upload_model->setItem(i, static_cast<int>(LocalListHeaderColumn::FILE_TIME),
                                         new QStandardItem(file_time.c_str()));
            m_file_upload_model->item(i, static_cast<int>(LocalListHeaderColumn::FILE_TIME))->setTextAlignment(
                    Qt::AlignCenter);
            m_file_upload_model->setItem(i, static_cast<int>(LocalListHeaderColumn::FILE_PATH),
                                         new QStandardItem(file_path.c_str()));
            m_file_upload_model->item(i, static_cast<int>(LocalListHeaderColumn::FILE_PATH))->setTextAlignment(
                    Qt::AlignCenter);
        }
        fout.close();
    }
}

void MainWindow::slot_updateDownloadRecords() {

    std::fstream fout(m_download_score_file_path.c_str(), std::ios::in);
    if (fout.is_open()) {
        int records_num = 0, model_num = m_file_download_model->rowCount();
        if (records_num < model_num) {
            m_file_download_model->removeRows(records_num - 1, m_file_download_model->rowCount() - records_num);
        }
        string file_name, file_size, file_path, file_time;
        for (int i = 0; fout >> file_name >> file_size >> file_path >> file_time; ++i) {
            file_time.replace(file_time.find('_'), 1, " ");
            m_file_download_model->setItem(i, static_cast<int>(LocalListHeaderColumn::FILE_NAME),
                                           new QStandardItem(file_name.c_str()));
            m_file_download_model->item(i, static_cast<int>(LocalListHeaderColumn::FILE_NAME))->setTextAlignment(
                    Qt::AlignCenter);
            m_file_download_model->setItem(i, static_cast<int>(LocalListHeaderColumn::FILE_SIZE),
                                           new QStandardItem(file_size.c_str()));
            m_file_download_model->item(i, static_cast<int>(LocalListHeaderColumn::FILE_SIZE))->setTextAlignment(
                    Qt::AlignCenter);
            m_file_download_model->setItem(i, static_cast<int>(LocalListHeaderColumn::FILE_TIME),
                                           new QStandardItem(file_time.c_str()));
            m_file_download_model->item(i, static_cast<int>(LocalListHeaderColumn::FILE_TIME))->setTextAlignment(
                    Qt::AlignCenter);
            m_file_download_model->setItem(i, static_cast<int>(LocalListHeaderColumn::FILE_PATH),
                                           new QStandardItem(file_path.c_str()));
            m_file_download_model->item(i, static_cast<int>(LocalListHeaderColumn::FILE_PATH))->setTextAlignment(
                    Qt::AlignCenter);
        }
        fout.close();
    }

}

void MainWindow::slot_message(QWidget *parent, const QString &title, const QString &text) {
    QMessageBox::information(parent, title, text);
}

MainWindow::~MainWindow() {
    if (m_local_upload_file.is_open()) {
        m_local_upload_file.close();
    }
    if (m_local_download_file.is_open()) {
        m_local_download_file.close();
    }
}

void MainWindow::slot_share_file() {
    bool ok;
    QString text = QInputDialog::getText(this, tr("分享文件"), tr("请输入接收人"), QLineEdit::Password,
                                         nullptr, &ok);
    if (ok && !text.isEmpty()) {
//                    LOG_INFO("接收人是：%s", text.toStdString().c_str())
        auto row = ui->fileTableView->currentIndex().row();
        QModelIndex id_index = m_file_list_model->index(row, FileListHeaderColumn::FILE_ID);
        map<string, string> data;
        data["id"] = m_file_list_model->data(id_index).toString().toStdString();
        data["username"] = text.toStdString();
        string res;
        auto req = std::make_shared<HTTPRequest>(g_config.ip.data(), g_config.port);
        req->init();
        map<string, string> headers;
        headers["token"] = g_config.token;
        int flag = req->post("/share", data, headers, res);
        if (flag) {
            Document document;
            document.Parse(res.c_str());
            if (document.IsObject() && document.HasMember("status") && document.HasMember("code")) {
                if (document["status"].GetInt() == 200 && document["code"].GetInt() == 1) {
                    emit sig_information(this, "成功", "分享成功");
                }
                else {
                    emit sig_information(this, "错误", document["msg"].GetString());
                }
            }
            else {
                emit sig_information(this, "错误", "请重试");
            }
        }
        req->close_socket();
    }
}
