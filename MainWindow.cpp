//
// Created by lisongqian on 2022/5/9.
//

#include <QFile>
#include <QDragEnterEvent>
//#include <QDropEvent>
#include <QMimeData>
#include <QCryptographicHash>
//#include <QDrag>
#include "MainWindow.h"
#include "request/HTTPRequest.h"    // winsock2.h 要在windows.h 前 locker.h中引用了windows.h
#include "log/log.h"
#include "config.h"
#include "rapidjson/document.h"

using namespace rapidjson;
extern Config g_config;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(std::make_shared<Ui::MainWindow>()), m_current_dir(0) {
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

    /*设置表格属性*/
    m_file_list_model = std::make_shared<QStandardItemModel>();
    m_file_list_model->setColumnCount(7);
    m_file_list_model->setHeaderData(0, Qt::Horizontal, "文件名");
    m_file_list_model->setHeaderData(1, Qt::Horizontal, "修改时间");
    m_file_list_model->setHeaderData(2, Qt::Horizontal, "类型");
    m_file_list_model->setHeaderData(3, Qt::Horizontal, "文件大小");
    m_file_list_model->setHeaderData(4, Qt::Horizontal, "路径");
    m_file_list_model->setHeaderData(5, Qt::Horizontal, "哈希值");
    m_file_list_model->setHeaderData(6, Qt::Horizontal, "类型值");
    ui->fileTableView->setModel(m_file_list_model.get());
    ui->fileTableView->verticalHeader()->hide();
    ui->fileTableView->setEditTriggers(QTableView::NoEditTriggers);
    ui->fileTableView->setShowGrid(false);
    ui->fileTableView->setColumnWidth(0, 220);
    ui->fileTableView->setColumnWidth(1, 150);
    ui->fileTableView->setColumnWidth(2, 50);
    ui->fileTableView->setColumnWidth(3, 80);
    ui->fileTableView->setColumnHidden(4, true);
    ui->fileTableView->setColumnHidden(5, true);
    ui->fileTableView->setColumnHidden(6, true);
    ui->fileTableView->setSelectionBehavior(QTableView::SelectRows);
    ui->fileTableView->setSelectionMode(QTableView::SingleSelection);
}

/**
 * 显示窗口
 */
void MainWindow::show_myself() {
    updateFileList();
    this->show();
}

/**
 * 连接信号，用于触发内部函数
 * @param dialog
 */
void MainWindow::addConnection(LoginDialog *dialog) const {
    connect(dialog, &LoginDialog::open_main_window, this, &MainWindow::show_myself);
}

/**
 * 切换标签页
 * @param currentRow
 */
void MainWindow::changeTab(int currentRow) {
    if (currentRow >= 0) {
        LOG_INFO("changeTab:%d", currentRow);
        ui->tabWidget->setCurrentIndex(currentRow);
        switch (currentRow) {
            case 0: {
                updateFileList();
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
    LOG_INFO("click index:%d", index)
    /**
     * index 为负数为 ">"，点击无效果
     */
    if (index < 0) return;
    while (m_navigation.size() > index * 2 + 1) {
        m_navigation.pop_back();
    }
    m_navigation[index * 2]->setProperty("level", "last");
    m_navigation[index * 2]->style()->polish(m_navigation[index * 2].get());
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

void MainWindow::addNavigation(std::string_view name) {
    QPoint point = m_navigation[m_navigation.size() - 1]->pos();
    auto tag = std::make_shared<ClickLabel>(-1, ui->navigation);
    QIcon icon = QIcon(":/images/caret-right.svg");
    QPixmap m_pic = icon.pixmap(icon.actualSize(QSize(10, 10)));//size自行调整
    tag->setPixmap(m_pic);
    tag->adjustSize();
    tag->setGeometry(QRect(point.x() + m_navigation[m_navigation.size() - 1]->width(), 0, tag->width(),
                           ui->navigation->height()));
    auto label = std::make_shared<ClickLabel>(tr(name.data()), (m_navigation.size() + 1) / 2, ui->navigation);
    label->adjustSize();
    label->setGeometry(
            QRect(point.x() + m_navigation[m_navigation.size() - 1]->width() + tag->width(), 0, label->width() + 8,
                  ui->navigation->height()));
    connect(label.get(), &ClickLabel::clicked, this, &MainWindow::navigationClick);
    label->setCursor(QCursor(Qt::PointingHandCursor));

    m_navigation[m_navigation.size() - 1]->setProperty("level", "normal");
    m_navigation[m_navigation.size() - 1]->style()->polish(m_navigation[m_navigation.size() - 1].get());
    label->setProperty("level", "last");
    label->style()->polish(label.get());
    m_navigation.push_back(std::move(tag));
    m_navigation.push_back(std::move(label));

}

void MainWindow::updateFileList() {
    string res;
    map<string, string> headers;
    headers["Token"] = g_config.token;
    map<string, string> data;
    data["dir"] = "0";
    auto req = std::make_shared<HTTPRequest>(g_config.ip.data(), g_config.port);
    req->init();
    bool flag = req->post("/filelist", data, headers, res);
    if (flag) {
        try {
            Document document;
            document.Parse(res.c_str());
            assert(document.IsObject());
            assert(document.HasMember("status"));
            assert(document.HasMember("code"));
            if (document["status"].GetInt() == 200 && document["code"].GetInt() == 1) {
                Value &list = document["data"];
                if (list.IsArray()) {
                    m_file_list_model->removeRows(0, m_file_list_model->rowCount());
                    for (int i = 0; i < list.Size(); ++i) {
                        Value &item = list[i];
                        if (item.IsObject()) {
                            m_file_list_model->setItem(i, 0, new QStandardItem(item["name"].GetString()));
                            m_file_list_model->setItem(i, 1, new QStandardItem(item["time"].GetString()));
                            if (strcmp(item["type"].GetString(), "0") == 0) {
                                m_file_list_model->setItem(i, 2, new QStandardItem("目录"));
                                m_file_list_model->setItem(i, 3, new QStandardItem("-"));
                            }
                            else {
                                m_file_list_model->setItem(i, 2, new QStandardItem("文件"));
                                m_file_list_model->setItem(i, 3, new QStandardItem(
                                        file_size_display(strtod(item["size"].GetString(), nullptr)).data()));
                            }
                            m_file_list_model->setItem(i, 4, new QStandardItem(item["addr"].GetString()));
                            m_file_list_model->setItem(i, 5, new QStandardItem(item["hash"].GetString()));
                            m_file_list_model->setItem(i, 6, new QStandardItem(item["type"].GetString()));
                            if (!i) {// i==0
                                m_current_dir = item["parent"].GetInt();
                            }
                        }
                    }
                }
            }
        }
        catch (std::exception &e) {
        }
    }
    req->close_socket();
}

void MainWindow::uploadFile(std::vector<std::shared_ptr<QFileInfo>> &files) {
    string res;
    map<string, string> headers;
    headers["Token"] = g_config.token;
    headers["Content-Type"] = "multipart/form-data; boundary=--boundary_yitiaohunxiaozifuchuanchuan";
    string mix_str = "------boundary_yitiaohunxiaozifuchuanchuan";
    map<string, string> data;

    string send_file_info, file_data;

    for (auto &item: files) {
        QFile localFile(item->filePath());
        send_file_info = mix_str + "\r\n";
        send_file_info += "Content-Disposition: form-data;";
        send_file_info += " parent=\"" + std::to_string(m_current_dir) + "\";";
        send_file_info += " name=\"" + item->fileName().toStdString() + "\";";
        if (!localFile.open(QFile::ReadOnly)) {
            LOG_ERROR("%s open error.", item->fileName().toStdString().c_str());
            continue;
        }
        QCryptographicHash ch(QCryptographicHash::Md5);

        quint64 totalBytes = localFile.size();
        quint64 bytesWritten = 0;
        quint64 bytesToWrite = totalBytes;
        quint64 loadSize = 1024 * 4;
        QByteArray buf;

        while (true) {
            if (bytesToWrite > 0) {
                buf = localFile.read(qMin(bytesToWrite, loadSize));
                file_data += buf.data();
                ch.addData(buf);
                bytesWritten += buf.length();
                bytesToWrite -= buf.length();
                buf.resize(0);
            }
            else {
                break;
            }
            if (bytesWritten == totalBytes) {
                break;
            }
        }
        localFile.close();
        auto sha1 = ch.result();
        send_file_info += " filename=\"" + sha1.toHex().toStdString() + "\"\r\n\r\n";
        send_file_info += file_data + "\r\n";
        file_data.clear();
    }
    send_file_info += mix_str + "--";
//    LOG_INFO("%s", send_file_info.c_str());
    data["data"] = send_file_info;
    auto req = std::make_shared<HTTPRequest>(g_config.ip.data(), g_config.port);
    req->init();
    bool flag = req->post("/upload", data, headers, res);
    if (flag) {
        LOG_INFO("%s", res.c_str());
    }
    req->close_socket();
}

void MainWindow::downloadFile() {
    string res;
    map<string, string> headers;
    headers["Token"] = g_config.token;
    map<string, string> data;
    data["filename"] = "0";
    data["id"] = "0";
    data["hash"] = "0";
    auto req = std::make_shared<HTTPRequest>(g_config.ip.data(), g_config.port);
    req->init();
    bool flag = req->post("/download", data, headers, res);
    if (flag) {

    }
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
    uploadFile(files);
}
