#ifndef COROUTINE_HPP
#define COROUTINE_HPP

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace threaded_coroutines {

class Coroutine {
  public:
    explicit Coroutine(std::function<void()> func);
    ~Coroutine();

    void wait();
    void yield();
    void resume();
    bool is_finished() const; // Function to check if finished
    void finish();

    static void add_coroutine(Coroutine *coroutine);
    static std::vector<Coroutine *> &get_coroutines();
    static std::mutex &get_global_mutex();
    static void cleanup();
    static Coroutine *current();

  private:
    std::function<void()> task_function;
    std::thread worker_thread;
    std::atomic<bool> finished_flag; // Renamed member variable
    bool is_yielding;
    std::mutex coroutine_mutex;
    std::condition_variable coroutine_cv;

    // Provides automatic cleanup at program exit
    struct StaticInitializer {
        StaticInitializer();
    };

    static StaticInitializer static_initializer;
    static thread_local Coroutine *current_coroutine;
};

// Helper function to yield the current coroutine
void yield();

} // namespace threaded_coroutines

#endif // COROUTINE_HPP
