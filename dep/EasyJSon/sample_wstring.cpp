/// Copyright 2015 hardcpp
/// https://github.com/hardcpp/EasyJSon
#include "EasyJSon.hpp"
#include <iostream>

void Write()
{
    EasyJSon::Node<std::wstring> l_Root;

     l_Root[L"ID"] = L"hello world";
     l_Root[L"Menu"][L"Item"][0][L"caption"] = L"test";
     l_Root[L"Menu"][L"Item"][0][L"link"]     = L"index.html";
     l_Root[L"Menu"][L"Item"][1][L"caption"] = L"forum";
     l_Root[L"Menu"][L"Item"][1][L"link"]     = L"forum.html";

     std::wcout << l_Root.Serialize<std::wostringstream>(true)  << std::endl;
}

int main()
{
    Write();
    system("pause");
    return 0;
}