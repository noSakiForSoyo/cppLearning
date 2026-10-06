#include <iostream>
#include <thread>
#include <mutex>

std::mutex m1;
std::mutex m2;

void t1(){
   std::scoped_lock lock(m1, m2); 
  std::cout << "Thread 1 acquired both locks" << std::endl;
}

void t2(){
  std::scoped_lock lock(m2, m1);
  std::cout << "Thread 2 acquired lock 2" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::lock_guard<std::mutex> lock1(m1);
  std::cout << "Thread 2 acquired both locks" << std::endl;
}

int main() {
  std::thread thread1(t1);
  std::thread thread2(t2);

  thread1.join();
  thread2.join();

  return 0;
}