#include "util.h"
#include <limits>
#include <ios>

#ifdef LINUX_OK_H
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#define OS_SEP ('/')
#endif

#ifdef WIN_OK_H
#include <io.h>
#include <direct.h>
#define OS_SEP ('\\')

// Win设置特殊显示或恢复原有设置，若reset为false，则设置特殊显示并原有的返回输出、输入设置码
// 若reset为true，则使用传入的输出、输入设置码设置显示，一般用于恢复原有设置，详见微软官网API
int set_color_cmd(DWORD &dwOriginalOutMode, DWORD &dwOriginalInMode, bool reset)
{
    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    if (reset)
    { //使用传入的设置码
        if ((!SetConsoleMode(hOut, dwOriginalOutMode)) || (!SetConsoleMode(hIn, dwOriginalInMode)))
        {
            return -1;
        }
    }
    else
    { //设置特殊显示并保存原有的设置码
        if (!GetConsoleMode(hOut, &dwOriginalOutMode))
        {
            return false;
        }
        if (!GetConsoleMode(hIn, &dwOriginalInMode))
        {
            return false;
        }
        if (!SetConsoleMode(hOut, dwOriginalOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
        {
            return -1; // Failed to set any VT mode
        }
        if (!SetConsoleMode(hIn, dwOriginalInMode | ENABLE_VIRTUAL_TERMINAL_INPUT))
        {
            return -1; // Failed to set VT input mode
        }
    }
    return 0;
}
#endif

void clean_cin()
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// 数字字符串左边补零直到指定位数n
// 例如s="35", n=4, 返回"0035"
std::string add_zero(const std::string &s, std::string::size_type n)
{
    std::string t(n, '0');
    t += s;
    return t.substr(t.length() - n);
}

// 检查文件或文件夹是否存在
// 存在返回0，否则返回非0值
int check_dir(const std::string &p)
{
#ifdef LINUX_OK_H
    return access(p.c_str(), F_OK);
#endif
#ifdef WIN_OK_H
    return _access(p.c_str(), 0);
#endif
    return -1;
}

// 新建文件夹，成功返回0，否则返回非0
int make_direct(const std::string &p)
{
#ifdef LINUX_OK_H
    return mkdir(p.c_str(), 0700);
#endif
#ifdef WIN_OK_H
    return _mkdir(p.c_str());
#endif
    return -1;
}

// 移去字符串首尾两端任意多个正反斜杠用于规范化路径
// 例如str="//aaa\\b/cc/ddd//\\"，返回"aaa\\b/cc/ddd"
std::string remove_chars(const std::string &str)
{
    std::string p(str);
    if (p.empty())
    {
        return p;
    }
    while (p.find_first_of("/\\") == 0) //开头存在斜杠
    {
        p = p.substr(1); //移去一个斜杠
    }
    if (p.empty())
    {
        return p;
    }
    while (p.find_last_of("/\\") + 1 == p.size()) //末尾存在斜杠
    {
        p = p.substr(0, p.size() - 1); //移去一个斜杠
    }
    return p;
}

// 用正斜杠或反斜杠连接路径
std::string join_path(std::initializer_list<std::string> path)
{
    std::string new_path;
    if ((*(path.begin())).find_first_of("/\\") == 0)
    {
        new_path += OS_SEP; //补回第一段路径开头的斜杠
    }
    bool check = false; //需要添加斜杠
    for (const auto &s : path)
    {
        const auto p = remove_chars(s); //消除首尾斜杠
        if (!p.empty())
        {
            if (check)
            {
                new_path += OS_SEP;
            }
            new_path += p;
            check = true;
        }
    }
    return new_path;
}

// 返回最后一段路径
std::string basename(const std::string &p)
{
    return p.substr(p.find_last_of("/\\") + 1);
}

// 检查ID均为数字，符合为true
bool check_str_id(const std::string &s)
{
    for (const auto &c : s)
    {
        if (c < '0' || c > '9')
        {
            return false;
        }
    }
    return true;
}
