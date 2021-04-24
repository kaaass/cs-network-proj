#include <memory>
#include <glog/logging.h>
#include "Events.h"

bool FdEventListener::attachTo(Epoll *epoll, uint32_t events) {
    return epoll->addFd(fd, events, {
            .ptr = this
    });
}

bool FdEventListener::modifyTo(Epoll *epoll, uint32_t events) {
    return epoll->modifyFd(fd, events, {
            .ptr = this
    });
}

[[noreturn]] void EpollEventReceiverThread::run() {
    LOG(INFO) << "EpollEventReceiverThread [" << getName() << "] started";
    //
    EventContext context {.thread = this};
    // 处理事件
    int cntEvent;
    while (true) {
        // 读入事件
        cntEvent = epoll->wait(eventBuffer);
        for (int i = 0; i < cntEvent; ++i) {
            auto event = static_cast<FdEventListener *>(eventBuffer[i].data.ptr);
            // 调用事件处理
            bool reserve = event->onEvent(context, eventBuffer[i].events);
            // 回收事件对象
            if (!reserve) {
                delete event;
            }
        }
    }
}

EpollEventReceiverThread::EpollEventReceiverThread(std::shared_ptr<Epoll> pEpoll) {
    eventBuffer = new epoll_event[pEpoll->getMaxEvents()];
    epoll = std::move(pEpoll);
}

EpollEventReceiverThread::~EpollEventReceiverThread() {
    delete[] eventBuffer;
    eventBuffer = nullptr;
}
