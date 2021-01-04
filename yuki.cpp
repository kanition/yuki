#include <stdio.h>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include "yuki.h"
#include "get_resp.h"
#include "biliparser.h"

#ifdef WIN_OK_H
#include <wchar.h>
#include <windows.h>

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
    {
        if ((!SetConsoleMode(hOut, dwOriginalOutMode)) || (!SetConsoleMode(hIn, dwOriginalInMode)))
        {
            return -1;
        }
    }
    else
    {
        if (!GetConsoleMode(hOut, &dwOriginalOutMode))
        {
            return false;
        }
        if (!GetConsoleMode(hIn, &dwOriginalInMode))
        {
            return false;
        }
        // we failed to set both modes, try to step down mode gracefully.
        if (!SetConsoleMode(hOut, dwOriginalOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
        {
            return -1; // Failed to set any VT mode, can't do anything here.
        }
        if (!SetConsoleMode(hIn, dwOriginalInMode | ENABLE_VIRTUAL_TERMINAL_INPUT))
        {
            return -1; // Failed to set VT input mode, can't do anything here.
        }
    }
    return 0;
}
#endif

int main(int argc, char const *argv[])
{
#ifdef WIN_OK_H
    DWORD dwOriginalOutMode = 0, dwOriginalInMode = 0;
    int color_flag = set_color_cmd(dwOriginalOutMode, dwOriginalInMode, false);
    if (color_flag)
    {
        std::cout << "No color" << std::endl;
    }
#endif
    std::cout << "Hello Yuki!" << std::endl;
#ifdef YUKI_VERSION_MAJOR
    std::cout << YUKI_VERSION_MAJOR << "." << YUKI_VERSION_MINOR << std::endl;
#endif
    std::string user_id;
    std::cout << "输入用户ID:" << std::endl;
    std::cin >> user_id;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    BiliAlbumParser bl(user_id);
    std::cout << "是否设置搜索时间段? y/[n]" << std::endl;
    char c = 'y';
    c = std::cin.get();
    c = std::cin.get();
    if (c == 'y')
    {
        bl.set_time();
    }
    std::string save_path;
    std::cout << "输入保存路径: " << std::endl;
    std::cin >> save_path;
    bl.parse(save_path);
    curl_global_cleanup();
#ifdef WIN_OK_H
    if (!color_flag)
    {
        set_color_cmd(dwOriginalOutMode, dwOriginalInMode, true);
    }
#endif
    return 0;
}
