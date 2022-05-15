#include <array>
#include <atomic>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

// 常量定义
const auto bufSiz = 8;
const auto threadNum = 10;

std::array<int, bufSiz> buf = {0};
std::vector<std::thread> thread_pool;

// sleep 函数
void sleep() {
  std::chrono::milliseconds timespan(200); // or whatever
  std::this_thread::sleep_for(timespan);
}

// 使用 c++ 原子变量模拟信号量
using atomic_int = std::atomic<int>;
atomic_int rw_mutex = {0};
atomic_int consumer_counter_mutex = {0};
int consumer_counter = 0;
void inc_atomic_relaxed(atomic_int &atomic_val) {
  atomic_val.fetch_add(1/* , std::memory_order_relaxed*/);
}
void dec_atomic_relaxed(atomic_int &atomic_val) {
  atomic_val.fetch_sub(1/* , std::memory_order_relaxed */);
}
void acquire(atomic_int &mutex) {
  while (mutex > 0)
    ;
  inc_atomic_relaxed(mutex);
}
void exit(atomic_int &mutex) { dec_atomic_relaxed(mutex); }

void producer() {
  for (int i = 0; i < 10; i++) { // 多次写入操作捏
    acquire(rw_mutex);
    // 临界区 确保只有一个生产者
    for (auto &data : buf) {
      data++;
    }
    sleep();
    exit(rw_mutex);
  }
}
void consumer() {
  for (int i = 0; i < 10; i++) { // 多次读取操作捏
    acquire(consumer_counter_mutex);
    // 保护 consumer_counter 的临界区
    consumer_counter++;
    if (consumer_counter == 1) {
      // 第一个获取到读写锁的消费者上读写锁
      acquire(rw_mutex);
    }
    exit(consumer_counter_mutex);

    for (auto &data : buf) {
      std::cout << data << " ";
    }
    std::cout << std::endl;
    acquire(consumer_counter_mutex);
    // 保护 consumer_counter 的临界区
    consumer_counter--;
    if (consumer_counter == 0) {
      // 没有消费者了就释放读写锁
      exit(rw_mutex);
    }
    exit(consumer_counter_mutex);
    sleep(); 
  }
}

void start() {
  for (int i = 0; i < threadNum; i++) {
    std::thread producer_thread(producer);
    std::thread consumer_thread(consumer);
    thread_pool.push_back(std::move(producer_thread));
    thread_pool.push_back(std::move(consumer_thread));
  }

  for (auto &thread : thread_pool) {
    thread.join();
  }
}

int main() {
  start();
  return 0;
}
