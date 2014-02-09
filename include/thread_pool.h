#ifndef DOSSIER_THREAD_POOL_H
#define DOSSIER_THREAD_POOL_H

#include <deque>
#include <vector>
#include <thread>
#include <future>
/*
 * Allow threads to execute unhindered
 * while keeping full controll over the threads.
 */
template<class T>
class ThreadPool {
public:
  typedef T return_type;
  typedef std::packaged_task<return_type()> task_type;
private:
  std::deque<task_type> _task;

  std::vector<std::thread> _thread;
  std::mutex _task_mutex;

  bool _continue;
public:
  ThreadPool(int threads) : _thread(threads), _continue(true) {
    for(auto &t : _thread) {
      t = std::thread(&ThreadPool::_main, this);
    }
  }

  ~ThreadPool() {
    join();
  }

  std::future<return_type> push(task_type&& newTask) {
    auto future = newTask.get_future();

    std::lock_guard<std::mutex> lg(_task_mutex);
    _task.emplace_back(std::move(newTask));

    return future;
  }

  void join() {
    if(!_continue) return;

    _continue = false;
    for(auto &t : _thread) {
      t.join();
    }
  }
private:
  int _pop(task_type &task) {
    std::lock_guard<std::mutex> lg(_task_mutex);

    if(_task.empty()) {
      return -1;
    }

    task = std::move(_task.front());
    _task.pop_front();
    return 0;
  }
public:
  void _main() {
    std::chrono::milliseconds sleeper(10);

    task_type task;
    while(_continue) {
      if(_pop(task)) {
        std::this_thread::sleep_for(sleeper);
        continue;
      }
      task();
    }

    // Execute remaining tasks
    while(!_pop(task)) {
      task();
    }
  }
};

#endif
