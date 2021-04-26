#ifndef CS_NETWORK_DOWNLOADMANAGER_H
#define CS_NETWORK_DOWNLOADMANAGER_H

#include <atomic>
#include <mutex>
#include <set>
#include "../server/ThreadPool.hpp"
#include "../server/Thread.h"
#include "../network/Socket.h"
#include "../util/File.h"
#include "../server/Events.h"

// 片段下载进度
struct PartStatus {
    PartStatus(uint64_t offset, uint64_t progress, uint64_t size);

    uint64_t offset;
    uint64_t progress;
    uint64_t size;
};

class DownloadThread;

/**
 * 多线程下载管理
 */
class DownloadManager {
public:

    void init();

    bool downloadWait(const std::string &srcFile, const std::shared_ptr<File>& destFile, uint64_t size);

    friend class DownloadThread;

private:

    // 更新下载状态
    void updateDownloadStatus(PartStatus *part);

private:
    // 下载缓冲区
    std::vector<std::shared_ptr<ByteBuffer>> readBuffers;
    // 下载线程池
    std::unique_ptr<ThreadPool<DownloadThread>> threadPool;
    // 下载进度
    std::vector<PartStatus> parts;
    // 下载中段偏移表，修改时必须锁 partsLock
    std::set<uint64_t> usingOffsets;
    // 下载进度占用锁
    std::mutex partsLock;
    // 下载源文件路径
    std::string srcPath;
    // 目标文件路径
    std::string destPath;
    // 总下载进度
    std::atomic<uint64_t> totalProgress;
    // 总大小
    uint64_t totalSize;
};

/**
 * 下载线程
 */
class DownloadThread : public Thread {
public:
    explicit DownloadThread(DownloadManager *manager);

    void run() override;

    friend class DownloadManager;
private:
    // 下载部分文件
    void downloadPart();

private:
    // 下载管理
    DownloadManager *manager;
    // 缓冲区
    std::shared_ptr<ByteBuffer> readBuffer;
    // 状态
    PartStatus *part = nullptr;
    // 服务器连接
    std::shared_ptr<Socket> connSocket;
    // 线程私有目标文件
    std::shared_ptr<File> destFile;
};

#endif //CS_NETWORK_DOWNLOADMANAGER_H
