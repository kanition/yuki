#include "get_resp.h"
#include "nlohmann/json.hpp"
#include "img_group.h"
#include "yuki.h"
#ifdef WIN_OK_H
#include <errno.h>
#endif

static size_t write_descr(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

int download_img(const std::string &url, const std::string &save_path, const curl_slist *chunk)
{
    CURL *curl = nullptr;
    curl = curl_easy_init();
    int status = 1;
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
#ifdef DEBUG
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif
        // curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_descr);
        FILE *pagefile = nullptr;
#ifdef WIN_OK_H
        errno_t err = fopen_s(&pagefile, save_path.c_str(), "wb");
        if (err == 0)
#else
        pagefile = fopen(save_path.c_str(), "wb");
        if (pagefile)
#endif
        {
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, pagefile);
            curl_easy_perform(curl);
            fclose(pagefile);
            status = 0;
        }
        curl_easy_cleanup(curl);
    }
    else
    {
        std::cerr << "Initial fail: curl_easy_init() in download_img()" << std::endl;
    }
    return status;
}

curl_slist *base_chunk(const std::string &origin, const std::string &referer)
{
    curl_slist *chunk = NULL;
    chunk = curl_slist_append(chunk, "sec-ch-ua: Google Chrome 87");
    chunk = curl_slist_append(chunk, "sec-ch-ua-mobile: ?0");
    chunk = curl_slist_append(chunk, "sec-fetch-dest: empty");
    chunk = curl_slist_append(chunk, "sec-fetch-mode: cors");
    chunk = curl_slist_append(chunk, "sec-fetch-site: same-site");
    chunk = curl_slist_append(chunk, "user-agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.88 Safari/537.36");
    if (origin.length() > 0)
    {
        chunk = curl_slist_append(chunk, ("origin: " + origin).c_str());
    }
    if (referer.length() > 0)
    {
        chunk = curl_slist_append(chunk, ("referer: " + referer).c_str());
    }
    return chunk;
}

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    MemoryStruct *mem = static_cast<MemoryStruct *>(userp);
    char *ptr = static_cast<char *>(realloc(mem->memory, mem->size + realsize + 1));
    if (ptr == nullptr) /* out of memory! */
    {
        std::cout << "not enough memory (realloc returned NULL)" << std::endl;
        return 0;
    }
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

CURLcode perform_get(CURL *curl, const curl_slist *chunk, MemoryStruct &mem, const std::string &url)
{
    mem.memory = static_cast<char *>(malloc(1)); /* will be grown as needed by the realloc above */
    mem.size = 0;                                /* no data at this point */
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
#ifdef DEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, static_cast<void *>(&mem));
    return curl_easy_perform(curl);
}

int get_all_count(const char *s)
{
    nlohmann::json j = nlohmann::json::parse(s);
    int n = j["data"]["all_count"].get<int>();
    return n;
}

img_group get_img_group(const char *s, const std::string &one_doc_id)
{
    img_group g;
    g.doc_id = one_doc_id;
    nlohmann::json j = nlohmann::json::parse(s);
    j = j["data"]["item"];
    g.description = j["description"].get<std::string>();
    g.upload_time = j["upload_time"].get<std::string>();
    for (nlohmann::json &item : j["pictures"])
    {
        g.imgs.push_back(item["img_src"].get<std::string>());
    }
    return g;
}

std::vector<std::string> doc_list(const char *s)
{
    nlohmann::json j = nlohmann::json::parse(s);
    std::vector<std::string> doc_list;
    unsigned long long one_doc;
    for (nlohmann::json &item : j["data"]["items"])
    {
        one_doc = item["doc_id"].get<unsigned long long>();
        doc_list.push_back(std::to_string(one_doc));
    }
    return doc_list;
}

int write_comment(const std::string &txt_name, const std::string &upload_time, const std::string &description)
{
    int status = 1;
    std::ofstream file(txt_name.c_str());
    if (file)
    {
        file << upload_time << "\n"
             << description << std::endl;
        file.close();
        status = 0;
    }
    return status;
}