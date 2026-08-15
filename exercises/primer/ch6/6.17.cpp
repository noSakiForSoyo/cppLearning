#include <iostream>
#include <string>
#include <cctype>
bool hasUpper(const std::string &s);
int main(){
    std::string s{"qinmingdizHishangbugao"};
    if(hasUpper(s)){
        std::cout << "存在大写\n";
    }

}
bool hasUpper(const std::string &s){
    for(auto word :s){
        if(std::isupper(word)){
            return true;
        }  
    }
    return false;
}