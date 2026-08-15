#include <iostream>
#include <fstream>
int main(){
    std::ifstream istr("test.txt");
    if(!istr.is_open()){
        std::cout << "文件名错误\n";
    }
    std::string name;
    int a;
    int b;
    double c;
    while(istr >> name >> a >> b >> c){
        std::cout << name <<a << b << c <<'\n';
    }
    istr.close();
}