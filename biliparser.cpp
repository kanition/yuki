#include <algorithm>
#include <limits>
#include <chrono>
#include <thread>
#include "biliparser.h"
#include "get_resp.h"
#include "yuki.h"
#include "util.h"

#define TIME_PAUSE_MAJOR 1500
#define TIME_PAUSE_MINOR 200

BiliAlbumParser::~BiliAlbumParser() {}
BiliAlbumParser::BiliAlbumParser() : page_num(0), from_time("1900-00-00"), to_time("9999-99-99") {}
BiliAlbumParser::BiliAlbumParser(const BiliAlbumParser &b) : user_id(b.user_id), page_num(b.page_num), from_time(b.from_time), to_time(b.to_time) {}
BiliAlbumParser::BiliAlbumParser(const std::string &s) : page_num(0), from_time("1900-00-00"), to_time("9999-99-99")
{
    if (check_str_id(s))
    {
        user_id = s;
    }
    else
    {
        std::cerr << "(｡•́︿•̀｡) 查无此人, 一定是你搞错啦: " << s << std::endl;
    }
}

std::string BiliAlbumParser::get_user_id() const
{
    return user_id;
}

// 获取页面数量
int BiliAlbumParser::parse_page_num()
{
    CURL *curl = nullptr;
    curl = curl_easy_init();
    int status = 1;
    if (curl)
    {
        struct curl_slist *chunk = base_chunk("https://space.bilibili.com", "https://space.bilibili.com/");
        std::string url = "https://api.vc.bilibili.com/link_draw/v1/doc/upload_count?uid=" + user_id;
        struct MemoryStruct mem;
        CURLcode res = perform_get(curl, chunk, mem, url);
        if (res == CURLE_OK)
        {
            int code_or_page = get_all_count(mem.memory); //解析页面数量
            if (code_or_page < 0)
            {
                std::cerr << curl_easy_strerror(res)
                          << "Error: parse_page_num-code_or_page=" << code_or_page << "\n"
                          << "\n(⊙︿⊙) 好像被小电视发现了,待会儿再试吧" << std::endl;
            }
            else
            {
                page_num = code_or_page;
                status = 0;
            }
        }
        else
        {
            std::cerr << curl_easy_strerror(res) << "\nError: parse_page_num-perform_get"
                      << "\n(⊙︿⊙) 对不起,我似乎遇到了点麻烦" << std::endl;
        }
        free(mem.memory);
        curl_slist_free_all(chunk);
        curl_easy_cleanup(curl);
    }
    else
    {
        std::cerr << "Error: parse_page_num-curl_easy_init"
                  << "\n(⊙︿⊙) 对不起,我似乎遇到了点麻烦" << std::endl;
    }
    return status;
}

int BiliAlbumParser::get_page_num() const
{
    return page_num;
}

// 时间格式归一化YYYY-MM-DD
std::string format_time(unsigned year, unsigned month, unsigned day)
{
    std::string a_time;
    if (year > 9999)
    {
        std::cerr << "(╬￣皿￣) 你看看你输的年份: " << year << std::endl;
        year = 9999;
    }
    a_time += add_zero(std::to_string(year), 4) + "-";
    if (month > 12)
    {
        std::cerr << "(╬￣皿￣) 你看看你输的月份: " << month << std::endl;
        month = 12;
    }
    a_time += add_zero(std::to_string(month), 2) + "-";
    if (day > 31)
    {
        std::cerr << "(╬￣皿￣) 你看看你输的日期: " << day << std::endl;
        day = 31;
    }
    a_time += add_zero(std::to_string(day), 2);
    return a_time;
}

// 设置起止时间
void BiliAlbumParser::set_time()
{
    unsigned year, month, day;
    std::cout << "～(￣▽￣～) 输入*起始*年月日, 像这样用空格间隔开 2000 09 13: " << std::endl;
    std::cin >> year >> month >> day;
    from_time = format_time(year, month, day);
    clean_cin();
    std::cout << "(～￣▽￣)～ 输入*结束*年月日, 老规矩: " << std::endl;
    std::cin >> year >> month >> day;
    clean_cin();
    to_time = format_time(year, month, day);
}

// 获取一个页面上的多个动态id
std::vector<std::string> BiliAlbumParser::parse_page_doc_id(const struct curl_slist *chunk, const std::string &url)
{
    CURL *curl = nullptr;
    curl = curl_easy_init();
    std::vector<std::string> page_doc;
    if (curl)
    {
        struct MemoryStruct mem;
        CURLcode res = perform_get(curl, chunk, mem, url);
        if (res == CURLE_OK)
        {
            page_doc = doc_list(mem.memory);
            if (page_doc.empty())
            {
                std::cerr << curl_easy_strerror(res) << "\nError: parse_page_doc_id-perform_get"
                          << "\n(⊙︿⊙) 好像被小电视发现了,待会儿再试吧" << std::endl;
            }
        }
        else
        {
            std::cerr << curl_easy_strerror(res) << "\nError: parse_page_doc_id-perform_get"
                      << "\n(⊙︿⊙) 对不起,我似乎遇到了点麻烦" << std::endl;
        }
        free(mem.memory);
        curl_easy_cleanup(curl);
    }
    else
    {
        std::cerr << "Error: parse_page_doc_id-curl_easy_init"
                  << "\n(⊙︿⊙) 对不起,我似乎遇到了点麻烦" << std::endl;
    }
    return page_doc;
}

// 保存图片和说明
void BiliAlbumParser::parse_doc_id(const std::string &save_path)
{
    struct curl_slist *chunk = base_chunk("https://space.bilibili.com", "https://space.bilibili.com/");
    struct curl_slist *img_chunk = base_chunk("https://h.bilibili.com", "https://h.bilibili.com/");
    std::string url = "https://api.vc.bilibili.com/link_draw/v1/doc/doc_list?uid=" + user_id + "&page_size=30&biz=all&page_num=";
    std::chrono::milliseconds ps_mj(TIME_PAUSE_MAJOR), ps_mn(TIME_PAUSE_MINOR); //暂停时间
    std::vector<std::string> fail_doc;
    int n = 0, remain_doc = 0; //成功图片数量和状态码
    for (int i = 0; i < page_num; i++)
    {
        std::vector<std::string> page_doc = parse_page_doc_id(chunk, url + std::to_string(i));
        if (page_doc.empty())
        {
            remain_doc = 1;
            break; //解析出错结束大循环
        }
        for (auto &d : page_doc) //返回的动态按时间从晚到早排序
        {
            img_group g = parse_img_group(img_chunk, d); //一个动态下的若干图片
            if (g.imgs.empty())
            {
                remain_doc = 1;
                i = page_num; //解析出错结束大循环
                break;
            }
            std::this_thread::sleep_for(ps_mj); //暂停
            if (g.upload_time.substr(0, 10) > to_time)
            { //时间比较是字符串比较，前十位格式为YYYY-MM-DD
                std::cout << "\033[K跳过: " << g.upload_time << " 动态ID: " << d << "\r" << std::flush;
                continue; //还未进入时间段
            }
            if (g.upload_time.substr(0, 10) < from_time)
            {
                i = page_num; //结束大循环
                std::cout << "\033[K跳过: " << g.upload_time << " 动态ID: " << d << "\r" << std::flush;
                break; //不在时间段内，小循环结束
            }
            std::string doc_path = join_path(save_path, d); //动态的保存路径
            if ((check_dir(doc_path) && make_direct(doc_path)) ||
                (write_comment(join_path(doc_path, "description.txt"), g.upload_time, g.description)))
            { //动态路径不存在又新建失败，或是不能写入说明
                fail_doc.push_back(d);
                continue; //记录失败
            }
            for (auto &u : g.imgs)
            { //保存图片
                std::string m = join_path(doc_path, basename(u));
                if (check_dir(m) && download_img(u, m, img_chunk))
                {
                    fail_doc.push_back(d);
                    break;
                }
                n++;
                std::this_thread::sleep_for(ps_mn); //暂停
            }
            std::cout << "\033[K在下啦: " << g.upload_time << " 动态ID: " << d << " 图: " << g.imgs.size() << "/" << n << "\r" << std::flush;
        }
    }
    curl_slist_free_all(chunk);
    curl_slist_free_all(img_chunk);
    std::cout << "\033[K" << std::endl;
    if (fail_doc.size())
    { //输出失败列表
        std::cerr << "(´･ω･)ﾉ(._.`) 这几个动态ID失败了:" << std::endl;
        for (auto &d : fail_doc)
        {
            std::cout << d << std::endl;
        }
    }
    if (remain_doc)
    {
        std::cerr << "(´･ω･)ﾉ(._.`) 未完成全部页面解析" << std::endl;
    }
    if (n > 0)
    {
        std::cout << "(づ◡ど) 一共搞到" << n << "张好康的" << std::endl;
    }
}

//解析一个动态下的图片
img_group BiliAlbumParser::parse_img_group(const struct curl_slist *chunk, const std::string &one_doc_id)
{
    CURL *curl = nullptr;
    curl = curl_easy_init();
    img_group g;
    if (curl)
    {
        std::string url = "https://api.vc.bilibili.com/link_draw/v1/doc/detail?doc_id=" + one_doc_id;
        struct MemoryStruct mem;
        CURLcode res = perform_get(curl, chunk, mem, url);
        if (res == CURLE_OK)
        {
            g = get_img_group(mem.memory, one_doc_id);
            if (g.imgs.empty())
            {
                std::cerr << curl_easy_strerror(res) << "\nError: parse_img_group-get_img_group"
                          << "\n(⊙︿⊙) 好像被小电视发现了,待会儿再试吧" << std::endl;
            }
        }
        else
        {
            std::cerr << curl_easy_strerror(res) << "\nError: parse_img_group-perform_get"
                      << "\n(⊙︿⊙) 对不起,我似乎遇到了点麻烦" << std::endl;
        }
        free(mem.memory);
    }
    else
    {
        std::cerr << "Error: parse_img_group-curl_easy_init"
                  << "\n(⊙︿⊙) 对不起,我似乎遇到了点麻烦" << std::endl;
    }
    curl_easy_cleanup(curl);
    return g;
}

// 设置、解析和下载
void BiliAlbumParser::parse(const std::string &save_path)
{
    std::string user_path = join_path(save_path, user_id);
    if (check_dir(user_path) && make_direct(user_path))
    { //检查保存路径
        std::cerr << "(￣へ￣) 哼你骗我, 这里连文件夹都没建: " << save_path << std::endl;
        return;
    }
    else
    {
        if (!parse_page_num())
        {
            std::cout << "(<ゝω・)☆ 接受你的挑战\n"
                      << "保存路径: " << save_path << std::endl
                      << *this << "\n(๑╹◡╹)ﾉ 准备起飞? [y]/n" << std::endl;
            char c = std::cin.get();
            clean_cin();
            if (c == 'n' || c == 'N')
            {
                std::cout << "ヾ(^∀^)ﾉ 收工啦" << std::endl;
                return;
            }
            parse_doc_id(user_path);
        }
    }
}

// 打印设置信息
std::ostream &operator<<(std::ostream &os, const BiliAlbumParser &b)
{
    os << "用户编号: " << b.get_user_id() << "\n"
       << "开始时间: " << b.from_time << "\n"
       << "结束时间: " << b.to_time << "\n"
       << "检测页数: " << b.get_page_num();
    return os;
}
