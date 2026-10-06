#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
#include <queue>
#include <condition_variable>

std::queue<int> q;
std::mutex mx;
std::condition_variable cv;
bool done = false;
void producer()
{
  for (int i = 0; i < 10; i++)
  {
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      std::lock_guard<std::mutex> lock(mx);
      q.push(i);
    }
    cv.notify_one();
  }
  {
    std::lock_guard<std::mutex> lock(mx);
    done = true;
  }
  cv.notify_all();
}
void consumer(int id)
{
  while (true)
  {
    {
      std::unique_lock<std::mutex> lock(mx);
      cv.wait(lock, []()
              { return !q.empty() && done; });
      if (q.empty() || done)
      {
        return;
      }
      int v = q.front();
      q.pop();
      std::string s = "Consumer " + std::to_string(id) + " consumed value: " + std::to_string(v) + '\n';
      lock.unlock();
      std::cout << s;
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  }
}
int main()
{
  std::thread t1(producer);
  std::thread t2(consumer, 1);
  std::thread t3(consumer, 2);
  t1.join();
  t2.join();
  t3.join();
  return 0;
}