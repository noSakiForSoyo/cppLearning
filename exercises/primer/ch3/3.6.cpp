#include <iostream>
#include <string>

int main(){
    std::string a = "jkdsaoweqwe";
    std::cout << a <<"\n";
    for(auto &b : a){
        b='x';
    }
    std::cout << a << "\n";
    std::string s;
    std::cout << s[0] << '\n';
    return 0;
    
}