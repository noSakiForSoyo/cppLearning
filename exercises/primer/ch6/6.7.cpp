#include <iostream>
#include "6.7.h"

int main(){
    for(int i = 0; i<10;i++){
        std::cout << getSize() << '\n';
    }
    return 0;
}
size_t getSize(){
    static size_t cnt= 0;
    return ++cnt;
}