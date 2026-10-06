#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
int main()
{
  std::mutex mtx;
  std::thread t1([&]()
                 {
      std::this_thread::sleep_for(std::chrono::seconds(10));
                  {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "Hello from thread!" 
          <<std::thread::hardware_concurrency()
          << std::endl;
                 } });
  std::thread t2([&]()
                 {
      std::this_thread::sleep_for(std::chrono::seconds(10));
                 {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "Hello from thread!" 
          <<std::thread::hardware_concurrency()
          << std::endl;
              } });
  std::thread t3([&]()
                 {
                   std::this_thread::sleep_for(std::chrono::seconds(10)); 
                  {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "Hello from thread!" 
          <<std::thread::hardware_concurrency()
          << std::endl;
                 } });
  std::thread t4([&]()
                 {
                   std::this_thread::sleep_for(std::chrono::seconds(10)); 
                  {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "Hello from thread!" 
          <<std::thread::hardware_concurrency()
          << std::endl;
                 } });

  t1.join();
  t2.join();
  t3.join();
  t4.join();
  std::cout << "Thread 1 ID: " << t1.get_id() << std::endl;
  std::cout << "Thread 2 ID: " << t2.get_id() << std::endl;
  std::cout << "Thread 3 ID: " << t3.get_id() << std::endl;
  std::cout << "Thread 4 ID: " << t4.get_id() << std::endl;
  return 0;
}