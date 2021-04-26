#ifndef CS_NETWORK_THREADPOOL_HPP
#define CS_NETWORK_THREADPOOL_HPP

#include <glog/logging.h>
#include <vector>
#include <memory>

/**
 * 线程池
 */
template<typename T>
class ThreadPool {
public:

    template<typename... Args>
    explicit ThreadPool(const std::string& name, int n, Args&&... _args) {
        threads.resize(n);
        for (int i = 0; i < n; ++i) {
            threads[i] = std::make_unique<T>(std::forward<Args>(_args)...);
            threads[i]->setName(name + "#" + std::to_string(i));
        }
    }

    std::vector<std::unique_ptr<T>> &lists() {
        return threads;
    }

    bool start() const {
        for (auto &thread : threads) {
            if (!thread->start()) {
                PLOG(ERROR) << "Cannot start thread " << thread->getName();
                return false;
            }
        }
        return true;
    }

    bool joinAll() const {
        int ret = true;
        for (auto &thread : threads) {
            if (!thread->join()) {
                PLOG(INFO) << "Cannot join thread " << thread->getName();
                ret = false;
            }
        }
        return ret;
    }

private:
    std::vector<std::unique_ptr<T>> threads;
};

#endif //CS_NETWORK_THREADPOOL_HPP
