/**
 * Created by lisongqian on 2022/3/30.
 * 跨平台完成 2022-4-7 15:40:13
 */

#ifndef NETDISK_COMMON_H
#define NETDISK_COMMON_H

#include <string>
#include <map>

using std::string;

constexpr std::uint32_t hash_str_to_uint32(const char *data) {
    std::uint32_t h(0);
    for (int i = 0; data && ('\0' != data[i]); i++)
        h = (h << 6) ^ (h >> 26) ^ data[i];
    return h;
}

static string Map2String(const std::map<string, string> &m) {
    string result = "{";
    char tmp[1024];
    memset(tmp, '\0', sizeof(tmp));
    for (auto& item: m) {
        snprintf(tmp,sizeof (tmp)-1,"%s:'%s',",item.first.c_str(),item.second.c_str());
        result += tmp;
        memset(tmp, '\0', sizeof(tmp));
    }
    result[result.length()-1] = '}';
    return result;
}

enum alertLevel {
    E_DEBUG = 0,
    E_INFO,
    E_WARN,
    E_ERROR
};
#endif //NETDISK_COMMON_H
