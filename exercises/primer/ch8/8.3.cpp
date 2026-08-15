#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <sstream>
int main(){
    std::string s;
    std::ofstream out("a.txt",std::ios::app);
    std::time_t now = std::time(nullptr);
    s = std::ctime(&now);
    std::ostringstream oss;
    oss << s.substr(0,24)<<'\n';
    out << oss.str();
    return 0;
}