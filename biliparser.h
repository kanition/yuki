#ifndef BILIALBUM
#define BILIALBUM
#include <iostream>
#include <vector>
#include <string>
#include <initializer_list>
#include "img_group.h"

class BiliAlbumParser
{
    friend std::ostream &operator<<(std::ostream &os, const BiliAlbumParser &b);

private:
    std::string user_id;
    int page_num;
    std::string from_time;
    std::string to_time;
    void parse_page_num();
    std::vector<std::string> parse_page_doc_id(const struct curl_slist *, const std::string &);
    void parse_doc_id(const std::string &);
    img_group parse_img_group(const struct curl_slist *, const std::string &);

public:
    ~BiliAlbumParser();
    BiliAlbumParser();
    BiliAlbumParser(const std::string &);
    BiliAlbumParser(const BiliAlbumParser &);
    void set_time();
    int parse(const std::string &);

    std::string get_user_id() const;
    int get_page_num() const;
};
std::ostream &operator<<(std::ostream &os, const BiliAlbumParser &b);
int check_dir(const std::string &);
#endif