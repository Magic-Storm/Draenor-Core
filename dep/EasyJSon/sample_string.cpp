/// Copyright 2015 hardcpp
/// https://github.com/hardcpp/EasyJSon
#include "EasyJSon.hpp"
#include <iostream>

void Write()
{
    EasyJSon::Node<std::string> l_Root;

    l_Root["ID"] = "hello world";
    l_Root["Menu"]["Item"][0]["caption"]     = "test";
    l_Root["Menu"]["Item"][0]["link"]     = "index.html";
    l_Root["Menu"]["Item"][1]["caption"]     = "forum";
    l_Root["Menu"]["Item"][1]["link"]     = "forum.html";

    std::cout << l_Root.Serialize<std::ostringstream>(true)  << std::endl;
}

int main()
{
    Write();
    system("pause");
    return 0;
}