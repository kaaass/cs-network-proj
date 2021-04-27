#include "Thread.h"

bool Thread::start() {
    return pthread_create(&handle,
                          nullptr,
                          reinterpret_cast<void *(*)(void *)>(Thread::runThread),
                          this) == 0;
}

void Thread::runThread(void *args) {
    auto self = static_cast<Thread *>(args);
    self->run();
}

bool Thread::join() const {
    return pthread_join(handle, nullptr) != 0;
}

const std::string &Thread::getName() const {
    return name;
}

void Thread::setName(const std::string &pName) {
    name = pName;
}

pthread_t Thread::getHandle() const {
    return handle;
}
