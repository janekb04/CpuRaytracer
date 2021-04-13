//
// Created by Jan on 13-Apr-21.
//

#ifndef ENGINE_DUPLEX_H
#define ENGINE_DUPLEX_H

#include <mutex>
#include <condition_variable>

class duplex {
private:
    bool sub_thread_is_working = false;
    std::mutex m;
    std::condition_variable cv;
public:
    void release() {
        {
            std::lock_guard<std::mutex> lk(m);
            sub_thread_is_working = true;
        }
        cv.notify_one();
    }

    void acquire() {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [&] { return !sub_thread_is_working; });
    }

    void lock() {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [&] { return sub_thread_is_working; });
        lk.release();
    }
    void unlock() {
        sub_thread_is_working = false;
        m.unlock();
        cv.notify_one();
    }
};

#endif //ENGINE_DUPLEX_H
