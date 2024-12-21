#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace threaded_coroutines {
class Coroutine {
  public:
    Coroutine(std::function<void()> func) : finished_(false), yield_(false) {
        // Wrap the original function to handle yielding and finishing
        func_ = [this, func]() {
            current_coroutine_ = this;    // Set thread-local pointer
            func();                       // Execute the task function
            finish();                     // Automatically mark as finished when function completes
            resume();                     // Wake up anyone waiting
            current_coroutine_ = nullptr; // Clear thread-local pointer
        };

        // Automatically start the thread for the coroutine
        worker_ = std::thread([this] { this->func_(); });

        // Add the coroutine to the global list
        addCoroutine(this);
    }

    ~Coroutine() {
        if (worker_.joinable()) {
            worker_.join(); // Ensure the thread is finished before destruction
        }
    }

    void wait() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !yield_ || finished_; });
    }

    void yield() {
        std::lock_guard<std::mutex> lock(mutex_);
        yield_ = true;
        cv_.notify_one(); // Notify manager that we're yielding
    }

    void resume() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            yield_ = false;
        }
        cv_.notify_one();
    }

    bool is_finished() const {
        return finished_;
    }

    void finish() {
        finished_ = true;
    }

    static void addCoroutine(Coroutine *coroutine) {
        std::lock_guard<std::mutex> lock(getGlobalMutex());
        getCoroutines().push_back(coroutine);
    }

    static std::vector<Coroutine *> &getCoroutines() {
        static std::vector<Coroutine *> coroutines;
        return coroutines;
    }

    static std::mutex &getGlobalMutex() {
        static std::mutex global_mutex;
        return global_mutex;
    }

    static void cleanup() {
        std::lock_guard<std::mutex> lock(getGlobalMutex());
        getCoroutines().clear(); // Automatically clean up all coroutines
    }

    // Provides access to the thread-local pointer to the current coroutine
    static Coroutine *current() {
        return current_coroutine_;
    }

  private:
    std::function<void()> func_;
    std::thread worker_;
    std::atomic<bool> finished_;
    bool yield_;
    std::mutex mutex_;
    std::condition_variable cv_;

    // Add a static initializer to register cleanup with std::atexit
    struct StaticInitializer {
        StaticInitializer() {
            std::atexit(Coroutine::cleanup);
        }
    };

    static StaticInitializer static_initializer_;

    // Thread-local pointer to the current coroutine
    static thread_local Coroutine *current_coroutine_;
};

// Define the static thread-local variable and the static initializer instance
thread_local Coroutine *Coroutine::current_coroutine_ = nullptr;
Coroutine::StaticInitializer Coroutine::static_initializer_;

// Helper function to yield the current coroutine
void yield() {
    if (Coroutine::current()) {
        Coroutine::current()->yield();
    }
}
}; // namespace threaded_coroutines
