#ifndef GET_RESP
#define GET_RESP
#include <iostream>
#include <vector>
#include <stdio.h>
#include <fstream>
#include "curl/curl.h"
#include "img_group.h"
struct MemoryStruct
{
    char *memory;
    size_t size;
};

int download_img(const std::string &, const std::string &, const curl_slist *);
curl_slist *base_chunk(const std::string &origin, const std::string &referer);
CURLcode perform_get(CURL *curl, const curl_slist *chunk, MemoryStruct &mem, const std::string &url);
int get_all_count(const char *);
std::vector<std::string> doc_list(const char *);
img_group get_img_group(const char *s, const std::string &one_doc_id);
int write_comment(const std::string &, const std::string &, const std::string &);
#endif