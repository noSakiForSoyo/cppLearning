#include <iostream>
#include <string>

int main(){
    std::string b; 
    std::getline(std::cin,b);
    std::cout << b <<"\n";

    std::string word;
    while(std::cin >> word){
        std::cout << word <<"\n";
    }
    return 0;
}