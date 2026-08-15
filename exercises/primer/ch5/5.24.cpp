#include <iostream>

int main(){
    int a,b;
    std::cin >> a >> b;
    try{
    if(b==0){
        throw std::runtime_error("除数不能为0");
    }
    std::cout << a/b << '\n';
    }catch(std::exception err){
        std::cout << err.what() << '\n';
    }
    return 0;
}