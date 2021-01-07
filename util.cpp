#include "util.h"
#include <limits>
#include <ios>
#include <vector>

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

// Win设置特殊显示或恢复原有设置，若reset为false，则设置特殊显示并原有的返回输出设置码
// 若reset为true，则使用传入的输出设置码设置显示，一般用于恢复原有设置，详见微软官网API
// https://docs.microsoft.com/zh-cn/windows/console/console-virtual-terminal-sequences#screen-colors
int set_color_cmd(DWORD &dwOriginalOutMode, bool reset)
{
    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    if (reset)
    { //使用传入的设置码
        if (!SetConsoleMode(hOut, dwOriginalOutMode))
        {
            return 1;
        }
    }
    else
    { //设置特殊显示并保存原有的设置码
        if (!GetConsoleMode(hOut, &dwOriginalOutMode))
        {
            return false;
        }
        if (!SetConsoleMode(hOut, dwOriginalOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
        {
            return 1; // Failed to set any VT mode
        }
    }
    return 0;
}
#endif

//清空缓冲区待读入输入
void clean_cin()
{
    std::cin.clear();
    std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
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
    return 1;
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
    return 1;
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

int check_save_path(std::string &path)
{
    return check_dir(path) && make_direct(path);
}

// 检查string均为数字
int check_str_id(const std::string &s)
{
    for (const auto &c : s)
    {
        if (c < '0' || c > '9')
        {
            std::cerr << "\n(Ｔ▽Ｔ) 查无此人, 一定是你搞错啦: " << s << std::endl;
            return 1;
        }
    }
    return 0;
}

int check_user_id(const std::string &s)
{
    int flag = check_str_id(s);
    if (flag)
    {
        std::cerr << "\n(Ｔ▽Ｔ) 查无此人, 一定是你搞错啦: " << s << std::endl;
    }
    return flag;
}

void now_time(unsigned &year, unsigned &month, unsigned &day)
{
    std::time_t t = std::time(0); // 当前时间
    std::tm *now = std::localtime(&t);
    year = now->tm_year + 1900;
    month = now->tm_mon + 1;
    day = now->tm_mday;
}

// 时间检查
int format_time(unsigned &year, unsigned &month, unsigned &day)
{
    int flag = 0;
    unsigned y, m, d, tmp;
    now_time(y, m, d);
    if (year > y)
    {
        flag = 1;
        year = y;
    }
    if (month < 1)
    {
        flag = 1;
        month = 1;
    }
    else
    {
        tmp = (year == y) ? m : 12;
        if (month > tmp)
        {
            flag = 1;
            month = tmp;
        }
    }
    if (day < 1)
    {
        flag = 1;
        day = 1;
    }
    else
    {
        tmp = (year == y && month == m) ? d : 31;
        if (day > tmp)
        {
            flag = 1;
            day = tmp;
        }
    }
    return flag;
}

int string_to_time(const std::string &input_time, std::string &output_time)
{
    if (input_time.length() != 8 || check_str_id(input_time))
    {
        return 1;
    }
    int year = std::stoi(input_time.substr(0, 4));
    int month = std::stoi(input_time.substr(4, 2));
    int day = std::stoi(input_time.substr(6, 2));
    unsigned y = year > 0 ? year : 0;
    unsigned m = month > 0 ? month : 0;
    unsigned d = day > 0 ? day : 0;
    return time_str(y, m, d, output_time);
}

// 时间格式归一化YYYY-MM-DD
int time_str(unsigned year, unsigned month, unsigned day, std::string &output_time)
{
    unsigned yyyy(year), mm(month), dd(day);
    int flag = format_time(yyyy, mm, dd);
    if (flag)
    {
        std::cerr << "(╬￣皿￣) 你看看你输的日期: " << year << " " << month << " " << day << std::endl;
    }
    output_time = add_zero(std::to_string(yyyy), 4) + "-";
    output_time += add_zero(std::to_string(mm), 2) + "-";
    output_time += add_zero(std::to_string(dd), 2);
    return flag;
}

// 输入时间
void input_time(std::string &from_time, std::string &to_time)
{
    unsigned year, month, day;
    std::cout << "～(￣▽￣～) 依次输入*起始*年月日, 可以每输一个数按一次Enter,\n"
              << "也可以像这样一口气输入哦 2000 09 13: " << std::endl;
    std::cin >> year >> month >> day;
    time_str(year, month, day, from_time);
    clean_cin();
    std::cout << "(～￣▽￣)～ 依次输入*结束*年月日, 老规矩: " << std::endl;
    std::cin >> year >> month >> day;
    clean_cin();
    time_str(year, month, day, to_time);
}

int option_parse(int argc, char const *argv[], std::string &user_id, std::string &save_path, std::string &from_time, std::string &to_time)
{
    std::vector<std::string> option(argv, argv + argc);
    for (int i = 1; i < argc; ++i)
    {
        const auto &opt = option[i];
        if (opt == "-u")
        {
            if (i + 1 < argc)
            {
                user_id = option[++i];
            }
            else
            {
                std::cerr << "\n-u参数未指定内容" << std::endl;
                return 1;
            }
        }
        else if (opt == "-s")
        {
            if (i + 1 < argc)
            {
                save_path = option[++i];
            }
            else
            {
                std::cerr << "\n-s参数未指定内容" << std::endl;
                return 1;
            }
        }
        else if (opt == "-t")
        {
            if (i + 2 < argc)
            {
                from_time = option[++i];
                to_time = option[++i];
            }
            else
            {
                std::cerr << "\n-t参数内容错误" << std::endl;
                return 1;
            }
        }
        else if (opt == "-h" || opt == "--help" || opt == "/?")
        {
            std::cout << "用法: yuki [-u 用户ID -s 保存路径 [-t 开始时间 结束时间]]\n"
                      << "  -u  指定用户ID, 均为数字\n"
                      << "  -s  指定保存路径, 必须为已存在的文件夹\n"
                      << "  -t  指定搜索时间段, 分别为年月日依次组成的8位数字YYYYMMDD"
                      << std::endl;
            return 2;
        }
        else
        {
            std::cerr << "\n错误参数: " << opt << std::endl;
            return 1;
        }
    }
    if (user_id.empty() || save_path.empty() || check_user_id(user_id))
    {
        std::cerr << "\n-u或-s参数错误" << std::endl;
        return 1;
    }
    return 0;
}