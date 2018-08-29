#include <iostream>
#include <fstream>
#include "hf.h"
#include <cstdio>
int main()
{
    std::cout << "请选择：\n";
    std::cout << "1.压缩\n" << "2.解压\n";
    int flag;
    std::cin >> flag;
    if(flag == 1)
    {
        std::cout << "请输入压缩文件路径及名称：";
        char path[1000];
        std::cin >> path;
        if(Compress(path))
            std::cout << "压缩完成!";
        else
            std::cout << "文件不存在!";
    }
    else if(flag == 2)
    {
        std::cout << "请输入解压缩文件路径及名称：";
        char path[1000];
        std::cin >> path;
        if(Decompress(path))
            std::cout << "解压完成!";
        else
            std::cout << "文件不存在!";
    }
    else
        std::cout << "选择错误\n";
    return 0;
}
