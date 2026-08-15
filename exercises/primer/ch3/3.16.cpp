#include <iostream>
#include <string>
#include <vector>
#include <cctype>

void toUpper(std::string &c){
    for(auto &word : c){
        word = std::toupper(word);
    }
}
int main(){
    std::string c;
    std::vector<std::string> s;
    while(std::cin >> c){
        toUpper(c);
        s.push_back(c);
        std::cout << c <<'\n';
    }
    
    return 0;
}
