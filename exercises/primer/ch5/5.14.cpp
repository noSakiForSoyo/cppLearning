#include <iostream>
#include <string>

int main(){
    std::string s,c;
    int cnt=1;
    while (std::cin >> s){
        if(!c.empty()&&s==c){
            cnt++;
        }
        if(!c.empty()&&c!=s){
            std::cout << c <<"出现次数: " << cnt << '\n';
            cnt = 1;
        }
        c=s;
    }
    return 0;
}