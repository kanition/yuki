#include <stdio.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <limits>
#include "yuki.h"
#include "get_resp.h"
#include "biliparser.h"
#include "util.h"

int main(int argc, char const *argv[])
{
#ifdef WIN_OK_H
    DWORD dwOriginalOutMode = 0;
    int color_flag = set_color_cmd(dwOriginalOutMode, false);
    if (color_flag)
    {
        std::cerr << "Warning: failed in setting virtual terminal" << std::endl;
    }
#endif
    //\x1b[38;2;<r>;<g>;<b>m设置颜色,\x1b[m恢复默认颜色
    std::cout << "\n\
\x1b[38;2;108;172;203m██╗   ██╗ ██╗   ██╗ \x1b[38;2;184;153;158m██╗  ██╗ \x1b[38;2;108;172;203m██╗\n\
\x1b[38;2;108;172;203m╚██╗ ██╔╝ ██║   ██║ \x1b[38;2;184;153;158m██║ ██╔╝ \x1b[38;2;108;172;203m██║\n\
\x1b[38;2;108;172;203m ╚████╔╝  ██║   ██║ \x1b[38;2;184;153;158m█████╔╝  \x1b[38;2;108;172;203m██║\n\
\x1b[38;2;108;172;203m  ╚██╔╝   ██║   ██║ \x1b[38;2;184;153;158m██╔═██╗  \x1b[38;2;108;172;203m██║\n\
\x1b[38;2;108;172;203m   ██║    ╚██████╔╝ \x1b[38;2;184;153;158m██║  ██╗ \x1b[38;2;108;172;203m██║\n\
\x1b[38;2;108;172;203m   ╚═╝     ╚═════╝  \x1b[38;2;184;153;158m╚═╝  ╚═╝ \x1b[38;2;108;172;203m╚═╝\x1b[m"
              << std::endl;
#ifdef YUKI_VERSION_MAJOR
    std::cout << "版本: " << YUKI_VERSION_MAJOR << "." << YUKI_VERSION_MINOR << std::endl;
#endif
    std::string user_id, save_path, from_time, to_time;
    int opt = 0;
    if (argc > 1)
    {
        opt = option_parse(argc, argv, user_id, save_path, from_time, to_time);
    }
    else
    {
        std::cout << "\n(*^▽^*) 你要看哪个Up的相册呢? 输入Ta的用户ID:" << std::endl;
        std::cin >> user_id;
        clean_cin();
        if (check_user_id(user_id))
        {
            opt = 1;
        }
        else
        {
            std::cout << "(o°ω°o) 翻翻黑历史? 是否设置时间段? y/[n]" << std::endl;
            char c;
            c = std::cin.get();
            clean_cin();
            if (c == 'y' || c == 'Y')
            {
                input_time(from_time, to_time);
            }
            std::cout << "(^o^)/ 最后一步啦! 输入保存路径: " << std::endl;
            std::cin >> save_path;
            clean_cin();
        }
    }
    while (!opt)
    {
        opt = 1;
        BiliAlbumParser bl(user_id);
        if (!from_time.empty() && !to_time.empty())
        {
            if (argc > 1)
            {
                if (bl.set_time(from_time, to_time, false))
                {
                    std::cerr << "-t参数错误: " << std::endl;
                    break;
                }
            }
            else
            {
                bl.set_time(from_time, to_time, true);
            }
        }
        CURLcode flag = curl_global_init(CURL_GLOBAL_DEFAULT); //全局初始化
        if (flag == CURLE_OK)
        {
            opt = bl.parse(save_path, argc - 1);
            curl_global_cleanup();
        }
        else
        {
            std::cerr << "\nError: main-curl_global_init" << std::endl;
        }
        break;
    }
    if (argc <= 1)
    {
        std::cout << "按Enter键退出" << std::endl;
        std::cin.get();
    }
#ifdef WIN_OK_H
    if (!color_flag)
    { //恢复显示设置
        set_color_cmd(dwOriginalOutMode, true);
    }
#endif
    return opt;
}
