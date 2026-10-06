#include <iostream>
#include "../include/math_util.h"

int main(){
  int a = 1;
  int b = 2;
  std::cout << "a + b = " << add(a, b) << std::endl;
  std::cout << "a * b = " << mul(a, b) << std::endl;
  return 0;
}