#include <stdio.h>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <limits>
#include "yuki.h"
#include "get_resp.h"
#include "biliparser.h"
#include "util.h"

int main(int argc, char const *argv[])
{
#ifdef WIN_OK_H
    DWORD dwOriginalOutMode = 0, dwOriginalInMode = 0;
    int color_flag = set_color_cmd(dwOriginalOutMode, dwOriginalInMode, false);
    if (color_flag)
    {
        std::cout << "Warning: failed in setting virtual terminal" << std::endl;
    }
#endif
    std::cout << "\n\
██╗   ██╗ ██╗   ██╗ ██╗  ██╗ ██╗\n\
╚██╗ ██╔╝ ██║   ██║ ██║ ██╔╝ ██║\n\
 ╚████╔╝  ██║   ██║ █████╔╝  ██║\n\
  ╚██╔╝   ██║   ██║ ██╔═██╗  ██║\n\
   ██║    ╚██████╔╝ ██║  ██╗ ██║\n\
   ╚═╝     ╚═════╝  ╚═╝  ╚═╝ ╚═╝"
              << std::endl;
#ifdef YUKI_VERSION_MAJOR
    std::cout
        << "版本: " << YUKI_VERSION_MAJOR << "." << YUKI_VERSION_MINOR << std::endl;
#endif
    std::string user_id;
    std::cout << "\n(*^▽^*) 你要看哪个Up的相册呢? 输入Ta的用户ID:" << std::endl;
    std::cin >> user_id;
    clean_cin();
    BiliAlbumParser bl(user_id);
    if (bl.get_user_id().empty())
    {
        return 1;
    }
    std::cout << "(o°ω°o) 翻翻黑历史? 是否设置时间段? y/[n]" << std::endl;
    char c = std::cin.get();
    clean_cin();
    if (c == 'y' || c == 'Y')
    {
        bl.set_time();
    }
    std::string save_path;
    std::cout << "٩(๑>◡<๑)۶ 最后一步啦! 输入保存路径: " << std::endl;
    std::cin >> save_path;
    clean_cin();
    curl_global_init(CURL_GLOBAL_DEFAULT); //全局初始化
    bl.parse(save_path);
    curl_global_cleanup();
#ifdef WIN_OK_H
    if (!color_flag)
    { //恢复显示设置
        set_color_cmd(dwOriginalOutMode, dwOriginalInMode, true);
    }
#endif
    return 0;
}
