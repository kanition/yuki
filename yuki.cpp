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

// Win设置特殊显示或恢复原有设置，若reset为false，则设置特殊显示并原有的返回输出、输入设置码
// 若reset为true，则使用传入的输出、输入设置码设置显示，一般用于恢复原有设置，详见微软官网API
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
    { //使用传入的设置码
        if ((!SetConsoleMode(hOut, dwOriginalOutMode)) || (!SetConsoleMode(hIn, dwOriginalInMode)))
        {
            return -1;
        }
    }
    else
    { //设置特殊显示并保存原有的设置码
        if (!GetConsoleMode(hOut, &dwOriginalOutMode))
        {
            return false;
        }
        if (!GetConsoleMode(hIn, &dwOriginalInMode))
        {
            return false;
        }
        if (!SetConsoleMode(hOut, dwOriginalOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
        {
            return -1; // Failed to set any VT mode
        }
        if (!SetConsoleMode(hIn, dwOriginalInMode | ENABLE_VIRTUAL_TERMINAL_INPUT))
        {
            return -1; // Failed to set VT input mode
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
        std::cout << "Warning: failed in setting virtual terminal" << std::endl;
    }
#endif
    std::cout << "Hello Yuki!" << std::endl;
#ifdef YUKI_VERSION_MAJOR
    std::cout << "Version: " << YUKI_VERSION_MAJOR << "." << YUKI_VERSION_MINOR << std::endl;
#endif
    std::string user_id;
    std::cout << "Input User ID:" << std::endl;
    std::cin >> user_id;
    BiliAlbumParser bl(user_id);
    if (bl.get_user_id().empty())
    {
        return 1;
    }
    std::cout << "Set Search Period? y/[n]" << std::endl;
    char c = std::cin.get();
    c = std::cin.get();
    if (c == 'y')
    {
        bl.set_time();
    }
    std::string save_path;
    std::cout << "Input Save path: " << std::endl;
    std::cin >> save_path;
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
