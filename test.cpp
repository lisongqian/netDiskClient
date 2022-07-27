//
// Created by lisongqian on 2022/5/16.
// encode: GBK
#include <iostream>
#include <string>
#include <utility>
#include <fstream>
#include <direct.h>
#include <QCryptographicHash>
#include <QFile>

using namespace std;

class father {
public:
    virtual void test()  {
        cout << "father test" << endl;
    }

};

class son : public father {
public:
    void test() override {
        cout << "son test" << endl;
    }
};

father *fun() {
    son *s = new son();
//    father *s = new father();
    return s;
}

int main() {
    father *f = fun();
    f->test();
//    cout << typeid(*f).name() << endl;
    auto s = dynamic_cast<father *>(f);
    cout << typeid(*s).name() << endl;
    s->test();
    delete f;
    return 0;
}

#if 0
void reference(int& v) {
    std::cout << " 左值引用" << std::endl;
}
void reference(int&& v) {
    std::cout << " 右值引用" << std::endl;
}
template <typename T>
void pass(T&& v) {
    std::cout << " 普通传参: ";
    reference(v);
    std::cout << " std::move 传参: ";
    reference(std::move(v));
    std::cout << " std::forward 传参: ";
    reference(std::forward<T>(v));
    std::cout << "static_cast<T&&> 传参: ";
    reference(static_cast<T&&>(v));
}
int main() {
    std::cout << " 传递右值:" << std::endl;
    pass(1);
    std::cout << " 传递左值:" << std::endl;
    int v = 1;
    pass(v);
    return 0;
}
#endif

#if 0
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
#endif
