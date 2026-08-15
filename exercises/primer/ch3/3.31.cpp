#include <iostream>
#include <vector>

const int MAX = 10;
int main(){
    int a[10];
    for(int i = 0;i<10;i++){
        a[i] = i;
    }
    for(auto b : a){
        std::cout << b <<'\n';
    }
    std::vector<int> c(10);
    for(auto &d : c){
        int i = 0;
        d=a[i];
        i++;
    }
    auto i = std::begin(a);
    for(auto b :a){
        std::cout << b << '\n';
    }
    return 0;
}