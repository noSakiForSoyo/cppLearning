#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
std::atomic<long> counter{0};

void work() {
    for (int i = 0; i < 1000; ++i) {
        ++counter;
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

int main(){
  std::vector<std::thread> threads;
  for(int i = 0; i < 4; ++i) {
    threads.emplace_back(work);
  }
  for(auto& t : threads) {
    t.join();
  }
  std::cout << "Final counter value: " << counter << std::endl;
  return 0;
}