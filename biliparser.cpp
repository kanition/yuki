#include <algorithm>
#include "biliparser.h"
#include "get_resp.h"
#include "yuki.h"

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
#endif

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
    std::vector<std::string> a;
    for (const auto &s : path)
    {
        a.push_back(remove_chars(s)); //消除首尾斜杠
    }
    std::string new_path;
    if ((*(path.begin())).find_first_of("/\\") == 0)
    {
        new_path += OS_SEP; //补回第一段路径开头的斜杠
    }
    bool check = false; //需要添加斜杠
    for (const auto &p : a)
    {
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

// 不定参数量模板重载使之也适合C字符串
template <typename... Ts>
std::string join_path(Ts &&... paths)
{
    static_assert(((std::is_same<typename std::decay<Ts>::type, std::string>::value ||
                    std::is_same<typename std::decay<Ts>::type, const char *>::value) ||
                   ...),
                  "T must be a basic_string");
    return join_path({paths...});
}

// 返回最后一段路径
std::string basename(const std::string &p)
{
    return p.substr(p.find_last_of("/\\") + 1);
}

// 检查ID均为数字，符合为true
bool check_str_id(const std::string &s)
{
    for (auto &c : s)
    {
        if (c < '0' || c > '9')
        {
            return false;
        }
    }
    return true;
}

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
        std::cerr << "Bad user id: " << s << std::endl;
    }
}

std::string BiliAlbumParser::get_user_id() const
{
    return user_id;
}

void BiliAlbumParser::parse_page_num()
{
    CURL *curl = nullptr;
    curl = curl_easy_init();
    if (curl)
    {
        struct curl_slist *chunk = base_chunk("https://space.bilibili.com", "https://space.bilibili.com/");
        std::string url = "https://api.vc.bilibili.com/link_draw/v1/doc/upload_count?uid=" + user_id;
        struct MemoryStruct mem;
        CURLcode res = perform_get(curl, chunk, mem, url);
        if (res == CURLE_OK)
        {
            page_num = get_all_count(mem.memory);
        }
        else
        {
            std::cerr << "perform_get() failed in BiliAlbumParser::parse_page_num(): " << curl_easy_strerror(res) << std::endl;
        }
        free(mem.memory);
        curl_slist_free_all(chunk);
        curl_easy_cleanup(curl);
    }
    else
    {
        std::cerr << "Initial fail: curl_easy_init() in BiliAlbumParser::parse_page_num()" << std::endl;
    }
}
int BiliAlbumParser::get_page_num() const
{
    return page_num;
}

std::string format_time(unsigned year, unsigned month, unsigned day)
{
    std::string a_time;
    if (year > 9999)
    {
        std::cerr << "Too big year num: " << year << std::endl;
        year = 9999;
    }
    a_time += add_zero(std::to_string(year), 4) + "-";
    if (month > 12)
    {
        std::cerr << "Too big month num: " << month << std::endl;
        month = 12;
    }
    a_time += add_zero(std::to_string(month), 2) + "-";
    if (day > 31)
    {
        std::cerr << "Too big day num: " << day << std::endl;
        day = 31;
    }
    a_time += add_zero(std::to_string(day), 2);
    return a_time;
}

void BiliAlbumParser::set_time()
{
    unsigned year, month, day;
    std::cout << "Input from time (eg. 2000 09 13): " << std::endl;
    std::cin >> year >> month >> day;
    from_time = format_time(year, month, day);
    std::cout << "Input to time (eg. 2019 07 18): " << std::endl;
    std::cin >> year >> month >> day;
    to_time = format_time(year, month, day);
    if (from_time > to_time)
    {
        std::string t(from_time);
        from_time = to_time;
        to_time = t;
    }
}

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
        }
        else
        {
            std::cerr << "perform_get() failed in BiliAlbumParser::parse_page_doc_id(): " << curl_easy_strerror(res) << std::endl;
        }
        free(mem.memory);
        curl_easy_cleanup(curl);
    }
    else
    {
        std::cerr << "Initial fail: curl_easy_init() in BiliAlbumParser::parse_page_doc_id()" << std::endl;
    }
    return page_doc;
}

void BiliAlbumParser::parse_doc_id(const std::string &save_path)
{
    struct curl_slist *chunk = base_chunk("https://space.bilibili.com", "https://space.bilibili.com/");
    struct curl_slist *img_chunk = base_chunk("https://h.bilibili.com", "https://h.bilibili.com/");
    std::string url = "https://api.vc.bilibili.com/link_draw/v1/doc/doc_list?uid=" + user_id + "&page_size=30&biz=all&page_num=";
    std::vector<std::string> fail_doc;
    int n = 0;
    for (int i = 0; i < page_num; i++)
    {
        std::vector<std::string> page_doc = parse_page_doc_id(chunk, url + std::to_string(i));
        for (auto &d : page_doc)
        {
            img_group g = parse_img_group(img_chunk, d);
            if (g.upload_time.substr(0, 10) > to_time)
            {
                continue;
            }
            if (g.upload_time.substr(0, 10) < from_time)
            {
                i = page_num;
                break;
            }
            std::string doc_path = join_path(save_path, d);
            if ((check_dir(doc_path) && make_direct(doc_path)) ||
                (write_comment(join_path(doc_path, "description.txt"), g.upload_time, g.description)))
            {
                fail_doc.push_back(d);
                continue;
            }
            for (auto &u : g.imgs)
            {
                std::string m = join_path(doc_path, basename(u));
                if (check_dir(m) && download_img(u, m, img_chunk))
                {
                    fail_doc.push_back(d);
                    break;
                }
                n++;
            }
            std::cout << "\033[KProcessing: " << g.upload_time << " doc_id: " << d << " img: " << g.imgs.size() << "/" << n << "\r" << std::flush;
        }
    }
    curl_slist_free_all(chunk);
    curl_slist_free_all(img_chunk);
    std::cout << "\033[K" << std::endl;
    if (fail_doc.size())
    {
        std::cout << "Fail doc id list:" << std::endl;
        for (auto &d : fail_doc)
        {
            std::cout << d << std::endl;
        }
    }
    std::cout << "Download " << n << " images" << std::endl;
}

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
        }
        else
        {
            std::cerr << "perform_get() failed in BiliAlbumParser::parse_img_group(): " << curl_easy_strerror(res) << std::endl;
        }
        free(mem.memory);
    }
    else
    {
        std::cerr << "Initial fail: curl_easy_init() in BiliAlbumParser::parse_img_group()" << std::endl;
    }
    curl_easy_cleanup(curl);
    return g;
}

void BiliAlbumParser::parse(const std::string &save_path)
{
    std::string user_path = join_path(save_path, user_id);
    if (check_dir(user_path) && make_direct(user_path))
    {
        std::cerr << "Bad save path: " << save_path << std::endl;
        return;
    }
    else
    {
        std::cout << "Success check save path: " << save_path << std::endl;
        std::cout << *this << std::endl;
        parse_page_num();
        parse_doc_id(user_path);
    }
}

std::ostream &operator<<(std::ostream &os, const BiliAlbumParser &b)
{
    os << "User: " << b.get_user_id() << "\n"
       << "From: " << b.from_time << "\n"
       << "  To: " << b.to_time << std::endl;
    return os;
}
