#ifndef CS_NETWORK_THREAD_H
#define CS_NETWORK_THREAD_H


#include <thread>

/**
 * 线程基类
 */
class Thread {

public:

    /**
     * 启动线程
     */
    bool start();

    /**
     * 合入子线程
     */
    bool join() const;

    /*
     * 线程主要逻辑，不适合直接调用
     */
    virtual void run() = 0;

    virtual ~Thread() = default;

private:

    // 运行线程的实际处理函数，分发消息给对应线程
    static void runThread(void *args);

private:
    pthread_t handle = 0;
};


#endif //CS_NETWORK_THREAD_H
