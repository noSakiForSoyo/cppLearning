#include <iostream>
#include <vector>

int main(){
    std::vector<int> i{1,2,3,4,5,6,7,8,9};
    auto it = i.begin();
    int a;
    while (it!=i.end()-1)
    {
        a=(*it);
        it++;
        std::cout << (*it)+a <<'\n';
    }
    return 0;

}