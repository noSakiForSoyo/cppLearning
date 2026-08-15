#include <iostream>
#include <fstream>
#include <string>

int main(){
    
    std::ifstream ifstem("test.txt");
    if(!ifstem.is_open()){
        std::cerr << "文件名错误\n";
        return -1;
    }
    std::string name;
    int a;
    int b;
    
    while(ifstem >> name >> a >>  b ){
        std::cout <<name << "总像素为："<< a*b<<'\n';
    }
    return 0;
}