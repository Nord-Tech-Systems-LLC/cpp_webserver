#ifndef COROUTINE_HPP
#define COROUTINE_HPP

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace threaded_coroutines {

class Coroutine {
  public:
    Coroutine(std::function<void()> func);
    ~Coroutine();

    void wait();
    void yield();
    void resume();
    bool is_finished() const;
    void finish();

    static void addCoroutine(Coroutine *coroutine);
    static std::vector<Coroutine *> &getCoroutines();
    static std::mutex &getGlobalMutex();
    static void cleanup();
    static Coroutine *current();

  private:
    std::function<void()> func_;
    std::thread worker_;
    std::atomic<bool> finished_;
    bool yield_;
    std::mutex mutex_;
    std::condition_variable cv_;

    struct StaticInitializer {
        StaticInitializer();
    };

    static StaticInitializer static_initializer_;
    static thread_local Coroutine *current_coroutine_;
};

// Helper function to yield the current coroutine
void yield();

} // namespace threaded_coroutines

#endif // COROUTINE_HPP
