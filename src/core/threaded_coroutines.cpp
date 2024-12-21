#include "cpp_webserver_include/core.hpp"
#include <atomic>
#include <functional>
#include <iostream>
#include <memory>

namespace threaded_coroutines {

thread_local Coroutine *Coroutine::current_coroutine_ = nullptr;
Coroutine::StaticInitializer Coroutine::static_initializer_;

Coroutine::StaticInitializer::StaticInitializer() {
    std::atexit(Coroutine::cleanup);
}

Coroutine::Coroutine(std::function<void()> func) : finished_(false), yield_(false) {
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

Coroutine::~Coroutine() {
    if (worker_.joinable()) {
        worker_.join(); // Ensure the thread is finished before destruction
    }
}

void Coroutine::wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !yield_ || finished_; });
}

void Coroutine::yield() {
    std::lock_guard<std::mutex> lock(mutex_);
    yield_ = true;
    cv_.notify_one(); // Notify manager that we're yielding
}

void Coroutine::resume() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        yield_ = false;
    }
    cv_.notify_one();
}

bool Coroutine::is_finished() const {
    return finished_;
}

void Coroutine::finish() {
    finished_ = true;
}

void Coroutine::addCoroutine(Coroutine *coroutine) {
    std::lock_guard<std::mutex> lock(getGlobalMutex());
    getCoroutines().push_back(coroutine);
}

std::vector<Coroutine *> &Coroutine::getCoroutines() {
    static std::vector<Coroutine *> coroutines;
    return coroutines;
}

std::mutex &Coroutine::getGlobalMutex() {
    static std::mutex global_mutex;
    return global_mutex;
}

void Coroutine::cleanup() {
    std::lock_guard<std::mutex> lock(getGlobalMutex());
    getCoroutines().clear(); // Automatically clean up all coroutines
}

Coroutine *Coroutine::current() {
    return current_coroutine_;
}

void yield() {
    if (Coroutine::current()) {
        Coroutine::current()->yield();
    }
}

} // namespace threaded_coroutines
