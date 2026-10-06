#include "myVecT.hpp"
#include "myVec.hpp"
#include "myVec_unique.hpp"
#include <thread>
int main(){
    myVecT<int> a;
    myVec b;
    b.push_back(223);
    myVec c(b);
    myVec_unique d;
    d.push_back(100);
    myVec_unique e = d;
    a.push_back(1);
    std::cout << a << b << c << '\n';
    std::thread t([&]{std::cout << d << e << '\n';});
    t.join();
}