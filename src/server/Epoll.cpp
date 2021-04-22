#include "Epoll.h"
#include <sys/epoll.h>
#include <glog/logging.h>

bool Epoll::init() {
    fd = epoll_create1(0);
    if (fd < 0) {
        return false;
    }
    return true;
}

Epoll::~Epoll() {
    if (fd != -1) {
        close(fd);
    }
}

bool Epoll::addFd(int pFd, uint32_t events) {
    return addFd(pFd, events, {.fd = pFd});
}

bool Epoll::addFd(int pFd, uint32_t events, void *data) {
    return addFd(pFd, events, {.ptr = data});
}

bool Epoll::addFd(int pFd, uint32_t events, epoll_data_t data) {
    epoll_event epollEvent{
            .events = events,
            .data = data,
    };

    return epoll_ctl(fd, EPOLL_CTL_ADD, pFd, &epollEvent) >= 0;
}

bool Epoll::modifyFd(int pFd, uint32_t events) {
    return modifyFd(pFd, events, {.fd = pFd});
}

bool Epoll::modifyFd(int pFd, uint32_t events, void *data) {
    return modifyFd(pFd, events, {.ptr = data});
}

bool Epoll::modifyFd(int pFd, uint32_t events, epoll_data_t data) {
    epoll_event epollEvent{
            .events = events,
            .data = data,
    };

    return epoll_ctl(fd, EPOLL_CTL_MOD, pFd, &epollEvent) >= 0;
}

int Epoll::wait(epoll_event *events, int timeLimit) const {
    return epoll_wait(fd, events, maxEvents, timeLimit);
}
