#ifndef CS_NETWORK_EVENTS_H
#define CS_NETWORK_EVENTS_H

#include <utility>

#include "Epoll.h"
#include "Thread.h"

/**
 * 事件上下文
 */
struct EventContext {
    Thread* thread;
};

/**
 * Epoll 事件侦听器
 */
class FdEventListener {

protected:

    explicit FdEventListener(int fd) : fd(fd) {}

    int fd;
public:

    /**
     * 事件发生时调用，事件会在线程对应的 eventhall 执行
     * @return 是否继续监听，若否则销毁事件
     */
    virtual bool onEvent(const EventContext &context, uint32_t events) = 0;

    bool attachTo(Epoll *epoll, uint32_t events);

    bool modifyTo(Epoll *epoll, uint32_t events);

    virtual ~FdEventListener() = default;
};

/**
 * Epoll 事件接受线程，实现 Reactor 模式
 */
class EpollEventReceiverThread : public Thread {
public:
    explicit EpollEventReceiverThread(std::shared_ptr<Epoll> epoll);

    void run() override;

    ~EpollEventReceiverThread() override;

private:
    std::shared_ptr<Epoll> epoll;

    epoll_event *eventBuffer = nullptr;
};

#endif //CS_NETWORK_EVENTS_H
