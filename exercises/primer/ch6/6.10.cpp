#include <iostream>

void swap(int *a,int *b){
    int c =*a;
    *a = *b;
    *b = c;
}

int main(){
    int a = 10;
    int b = 2;
    int *p1 = &a;
    int *p2 = &b;
    swap(p1,p2);
    std::cout << a << b <<'\n';

}