#include <array>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

// 常量定义
const auto bufSiz = 8;

std::array<int, bufSiz> buf = {0};
std::vector<std::thread> thread_pool;

// sleep 函数
void sleep(int sleep_time) {
  std::chrono::milliseconds timespan(sleep_time); // or whatever
  std::this_thread::sleep_for(timespan);
}

// 使用 c++ 原子变量模拟信号量
using atomic_int = std::atomic<int>;
atomic_int rw_mutex = {0};
atomic_int output_mutex = {0};
atomic_int consumer_counter_mutex = {0};
int consumer_counter = 0;
void inc_atomic_relaxed(atomic_int &atomic_val) {
  atomic_val.fetch_add(1 /* , std::memory_order_relaxed*/);
}
void dec_atomic_relaxed(atomic_int &atomic_val) {
  atomic_val.fetch_sub(1 /* , std::memory_order_relaxed */);
}
void acquire(atomic_int &mutex) {
  while (mutex > 0)
    ;
  inc_atomic_relaxed(mutex);
}
void exit(atomic_int &mutex) { dec_atomic_relaxed(mutex); }

void producer(int id) {
  acquire(rw_mutex);
  printf("Producer %d is started!\n", id); // 输出线程 id
  // 临界区 确保只有一个生产者
  for (int i = 0; i < bufSiz; i++)
    buf[i]++;

  printf("Producer changed the data\n");
  printf("Producer %d quitted!\n", id); // 输出线程 id
  exit(rw_mutex);
}
void consumer(int id) {
  acquire(consumer_counter_mutex);
  // 保护 consumer_counter 的临界区
  consumer_counter++;
  if (consumer_counter == 1) {
    // 第一个获取到读写锁的消费者上读写锁
    acquire(rw_mutex);
  }
  exit(consumer_counter_mutex);

  printf("Consumer %d is started!\n", id); // 输出线程 id
  // 输出内容
  acquire(output_mutex);
  // 阻止对缓冲区的并发输出
  for (auto &data : buf) {
    printf("%d ", data);
  }
  printf("\n");
  sleep(200);
  exit(output_mutex);
  printf("Consumer %d quitted!\n", id); // 输出线程 id

  acquire(consumer_counter_mutex);
  // 保护 consumer_counter 的临界区
  consumer_counter--;
  if (consumer_counter == 0) {
    // 没有消费者了就释放读写锁
    exit(rw_mutex);
  }
  exit(consumer_counter_mutex);
}

void start() {
  std::thread create_producer([]() {
    for (int i = 0; i < 3; i++) {
      std::thread producer_thread(producer, i);
      thread_pool.push_back(std::move(producer_thread));
      sleep(500);
    }
  });

  std::thread create_consumer([]() {
    for (int i = 0; i < 4; i++) {
      std::thread consumer_thread(consumer, i);
      thread_pool.push_back(std::move(consumer_thread));
      sleep(100);
    }
  });
  sleep(1000);
  std::thread create_consumer1([]() {
    for (int i = 0; i < 4; i++) {
      std::thread consumer_thread(consumer, 10 + i);
      thread_pool.push_back(std::move(consumer_thread));
      sleep(100);
    }
  });

  create_producer.join();
  create_consumer.join();
  create_consumer1.join();
  for (auto &thread : thread_pool) {
    thread.join();
  }

  puts("The final data is: ");
  for (auto &data : buf) {
    printf("%d ", data);
  }
  printf("\n");
}

int main() {
  start();
  return 0;
}
