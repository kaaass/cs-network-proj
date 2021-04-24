#ifndef CS_NETWORK_RIO_H
#define CS_NETWORK_RIO_H


#include <cstddef>

extern "C" {
size_t rio_writen(int fd, const char *usrbuf, size_t n);
}


#endif //CS_NETWORK_RIO_H
