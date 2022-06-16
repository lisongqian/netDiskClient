//
// Created by lisongqian on 2022/5/16.
//
#include <iostream>
#include <fstream>
#include <direct.h>
#include <QCryptographicHash>
#include <QFile>

using namespace std;

int main() {
    string file_name, file_data;
    file_name = "D:/Downloads/test.pdf";
    FILE *file = fopen(file_name.c_str(), "rb");
    if (file == nullptr) {
        printf("%s open error.\n", file_name.c_str());
        return -1;
    }
    QCryptographicHash ch(QCryptographicHash::Md5);
    QCryptographicHash ch_string(QCryptographicHash::Md5);

    size_t size;
    char buf[4096];

    while ((size = fread(buf, 1, 4096, file)) != 0) {
        ch.addData(buf, size);
        file_data += buf;
    }
    ch_string.addData(file_data.c_str());
    printf("char: %s\n", ch.result().toHex().toStdString().c_str());
    printf("string: %s\n", ch.result().toHex().toStdString().c_str());
    fclose(file);

    QCryptographicHash ch1(QCryptographicHash::Md5);

    QFile localFile(file_name.c_str());
    if (!localFile.open(QFile::ReadOnly)) {
        printf("%s open error.\n", file_name.c_str());
    }
    quint64 totalBytes = localFile.size();
    quint64 bytesWritten = 0;
    quint64 bytesToWrite = totalBytes;
    quint64 loadSize = 1024 * 4;
    QByteArray byteArray;

    while (true) {
        if (bytesToWrite > 0) {
            byteArray = localFile.read(qMin(bytesToWrite, loadSize));
            ch1.addData(byteArray);
            bytesWritten += byteArray.length();
            bytesToWrite -= byteArray.length();
            byteArray.resize(0);
        }
        else {
            break;
        }
        if (bytesWritten == totalBytes) {
            break;
        }
    }
    printf("QFile: %s\n", ch1.result().toHex().toStdString().c_str());
    localFile.close();

    char *new_str = new char[100];
    char static_str[100];
    cout << sizeof(static_str) <<" "<< sizeof(*static_str) << endl;
    cout << sizeof(new_str)<<" " << sizeof(*new_str) << endl;

    delete[] new_str;
}
