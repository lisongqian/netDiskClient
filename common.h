/**
 * Created by lisongqian on 2022/3/30.
 * 跨平台完成 2022-4-7 15:40:13
 */

#ifndef NETDISK_COMMON_H
#define NETDISK_COMMON_H

#include <map>
#include <cstring>

using std::string;
using std::map;

constexpr std::uint32_t hash_str_to_uint32(const char *data) {  // C++ 14以上支持,11不支持使用局部变量、循环和分支等简单语句
    std::uint32_t h(0);
    for (int i = 0; data && ('\0' != data[i]); i++)
        h = (h << 6) ^ (h >> 26) ^ data[i];
    return h;
}

static string file_size_display(double size) {
    char str[10];
    memset(str, '\0', sizeof(str));
    int count = 0;
    while (size > 1024 && count < 5) {
        size = (size * 1.0) / 1024;
        count++;
    }
    snprintf(str, 10, "%.1f", size);
    string result = str;
    switch (count) {
        case 0:
            result += "B";
            break;
        case 1:
            result += "KB";
            break;
        case 2:
            result += "MB";
            break;
        case 3:
            result += "GB";
            break;
        case 4:
            result += "TB";
            break;
        case 5:
            result += "PB";
            break;
        default:
            break;
    }
    return result;
}

enum class alertLevel : unsigned int {
    E_DEBUG = 0,
    E_INFO,
    E_WARN,
    E_ERROR
};

enum FileType {
    E_DIR = 0,
    E_FILE = 1,
};
#endif //NETDISK_COMMON_H
