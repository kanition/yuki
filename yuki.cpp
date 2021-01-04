#include <stdio.h>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include "yuki.h"
#include "get_resp.h"
#include "biliparser.h"

int main(int argc, char const *argv[])
{
    std::cout << "Hello Yuki!" << std::endl;
#ifdef YUKI_VERSION_MAJOR
    std::cout << YUKI_VERSION_MAJOR << "." << YUKI_VERSION_MINOR << std::endl;
#endif
    std::string user_id;
    std::cout << "Input user id:" << std::endl;
    std::cin >> user_id;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    BiliAlbumParser bl(user_id);
    std::cout << "set time or not? y/[n]" << std::endl;
    char c = 'y';
    c = std::cin.get();
    c = std::cin.get();
    if (c == 'y')
    {
        bl.set_time();
    }
    std::string save_path;
    std::cout << "Input save path: " << std::endl;
    std::cin >> save_path;
    bl.parse(save_path);
    curl_global_cleanup();
    return 0;
}
