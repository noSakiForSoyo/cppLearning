#include <iostream>

int main(){
    int ia[3][4];
    int index = 0;
    for(auto &p :ia){
        for(auto &q:p){

            q=index;
            index++;
        }
    }
    for(int i = 0;i<3;i++){
        for(int j = 0;j<4;j++){
            std::cout << ia[i][j] << ' ';
        }
        std::cout << '\n';
    }
    for(auto p = ia;p!=ia+3;p++){
        for(auto q = *p;q!=*p+4;q++){
            std::cout << *q << ' ';
        }
        std::cout << '\n';
    }
    return 0;
}