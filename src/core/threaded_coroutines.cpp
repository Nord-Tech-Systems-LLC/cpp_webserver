#include "cpp_webserver_include/core.hpp"
#include <atomic>
#include <functional>
#include <iostream>
#include <memory>

namespace threaded_coroutines {

thread_local Coroutine *Coroutine::current_coroutine = nullptr;
Coroutine::StaticInitializer Coroutine::static_initializer;

Coroutine::StaticInitializer::StaticInitializer() {
    std::atexit(Coroutine::cleanup);
}

Coroutine::Coroutine(std::function<void()> func) : finished_flag(false), is_yielding(false) {
    task_function = [this, func]() {
        current_coroutine = this;    // Set thread-local pointer
        func();                      // Execute the task function
        finish();                    // Automatically mark as finished when function completes
        resume();                    // Wake up anyone waiting
        current_coroutine = nullptr; // Clear thread-local pointer
    };

    // Automatically start the thread for the coroutine
    worker_thread = std::thread([this] { this->task_function(); });

    // Add the coroutine to the global list
    add_coroutine(this);
}

Coroutine::~Coroutine() {
    if (worker_thread.joinable()) {
        worker_thread.join(); // Ensure the thread is finished before destruction
    }
}

void Coroutine::wait() {
    std::unique_lock<std::mutex> lock(coroutine_mutex);
    coroutine_cv.wait(lock, [this] { return !is_yielding || finished_flag; });
}

void Coroutine::yield() {
    std::lock_guard<std::mutex> lock(coroutine_mutex);
    is_yielding = true;
    coroutine_cv.notify_one(); // Notify manager that we're yielding
}

void Coroutine::resume() {
    {
        std::lock_guard<std::mutex> lock(coroutine_mutex);
        is_yielding = false;
    }
    coroutine_cv.notify_one();
}

bool Coroutine::is_finished() const {
    return finished_flag;
}

void Coroutine::finish() {
    finished_flag = true;
}

void Coroutine::add_coroutine(Coroutine *coroutine) {
    std::lock_guard<std::mutex> lock(get_global_mutex());
    get_coroutines().push_back(coroutine);
}

std::vector<Coroutine *> &Coroutine::get_coroutines() {
    static std::vector<Coroutine *> coroutine_list;
    return coroutine_list;
}

std::mutex &Coroutine::get_global_mutex() {
    static std::mutex global_mutex;
    return global_mutex;
}

void Coroutine::cleanup() {
    std::lock_guard<std::mutex> lock(get_global_mutex());
    get_coroutines().clear(); // Automatically clean up all coroutines
}

Coroutine *Coroutine::current() {
    return current_coroutine;
}

void yield() {
    if (Coroutine::current()) {
        Coroutine::current()->yield();
    }
}

} // namespace threaded_coroutines
