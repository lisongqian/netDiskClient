//
// Created by lisongqian on 2022/5/20.
//

#ifndef NETDISK_CLICKLABEL_H
#define NETDISK_CLICKLABEL_H


#include <QLabel>

class ClickLabel : public QLabel {
Q_OBJECT
public:
    explicit ClickLabel(int index = -1, QWidget *parent = nullptr);

    explicit ClickLabel(const QString &text, int index = -1, QWidget *parent = nullptr,
                        Qt::WindowFlags f = Qt::WindowFlags());

    void init();
    ~ClickLabel() override = default;
    int m_dir_id;

signals:

    void clicked(int index);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    int m_index;        // 目录跳转等级、默认跳转到根目录
};


#endif //NETDISK_CLICKLABEL_H
