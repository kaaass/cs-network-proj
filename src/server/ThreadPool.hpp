#ifndef CS_NETWORK_THREADPOOL_HPP
#define CS_NETWORK_THREADPOOL_HPP


#include <vector>

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

    const std::vector<std::unique_ptr<T>> &lists() const {
        return threads;
    }

    bool start() const {
        for (auto &thread : threads) {
            LOG(INFO) << "Start thread: " << thread->getName();
            if (!thread->start()) {
                PLOG(ERROR) << "Cannot start thread " << thread->getName();
                return false;
            }
        }
        return true;
    }

private:
    std::vector<std::unique_ptr<T>> threads;
};

#endif //CS_NETWORK_THREADPOOL_HPP
