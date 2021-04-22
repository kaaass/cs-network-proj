#ifndef CS_NETWORK_EPOLL_H
#define CS_NETWORK_EPOLL_H

#include <cstdint>
#include <sys/epoll.h>

/**
 * 管理 Epoll
 */
class Epoll {
public:

    /**
     * 初始化
     */
    bool init();

    /**
     * 向 Epoll 增加监听的描述符
     */
    bool addFd(int fd, uint32_t events);

    bool addFd(int fd, uint32_t events, void *data);

    bool addFd(int fd, uint32_t events, epoll_data_t data);

    /**
     * 向 Epoll 更改描述符配置
     */
    bool modifyFd(int fd, uint32_t events);

    bool modifyFd(int fd, uint32_t events, void *data);

    bool modifyFd(int fd, uint32_t events, epoll_data_t data);

    /**
     * 监听 Epoll 事件，线程安全
     */
    int wait(epoll_event *events, int timeLimit = -1) const;

    virtual ~Epoll();

public:
    int fd = -1;
private:
    int maxEvents = 64;
};


#endif //CS_NETWORK_EPOLL_H
