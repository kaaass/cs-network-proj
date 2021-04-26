#include "DownloadManager.h"

#include <utility>
#include "Client.h"

#define READ_BUFFER_PER_THREAD (64 * 1048576)
#define PART_SIZE (32 * 1048576)

void DownloadManager::init() {
    // 初始化下载缓冲区
    for (int i = 0; i < Client::INSTANCE->downloadThreads; i++) {
        readBuffers.push_back(std::make_shared<ByteBuffer>());
        readBuffers[i]->allocate(READ_BUFFER_PER_THREAD);
    }
}

bool DownloadManager::downloadWait(const std::string &srcFile, const std::shared_ptr<File>& destFile, uint64_t size) {
    srcPath = srcFile;
    destPath = destFile->getRealPath();
    totalProgress = 0;
    totalSize = size;
    // 初始化线程池
    threadPool = std::make_unique<ThreadPool<DownloadThread>>(
            "DownloadPool", Client::INSTANCE->downloadThreads, this);
    auto &threads = threadPool->lists();
    for (int i = 0; i < threads.size(); i++) {
        threads[i]->readBuffer = readBuffers[i];
    }
    // 检查断点续传
    if (false) {
        // 继续下载
        // 读取下载状态文件
        // TODO
    } else {
        // 新下载
        // 预分配文件大小
        destFile->allocate(size);
        // 划分下载分块
        parts.clear();
        uint64_t curPos = 0;
        auto rest = size;
        while (rest >= PART_SIZE + (PART_SIZE >> 1)) {
            parts.emplace_back(curPos, 0, PART_SIZE);
            curPos += PART_SIZE;
            rest -= PART_SIZE;
        }
        if (rest > 0) {
            parts.emplace_back(curPos, 0, rest);
        }
        // 生成下载状态文件
        // TODO
    }
    // 准备下载
    bool ret = threadPool->start();
    if (!ret) {
        perror("Download failed! ");
    }
    // 等待下载结束
    threadPool->joinAll();
    // 清除线程池、下载分块信息
    parts.clear();
    threadPool = nullptr;
    // 删除下载状态文件
    // TODO
    return true;
}

void DownloadManager::updateDownloadStatus(PartStatus *part) {
    LOG(INFO) << "Download progress: " << double(totalProgress) / totalSize * 100 << "%";
    // TODO 状态文件
}

void DownloadThread::run() {
    // 打开线程本地用文件
    destFile = File::open(manager->destPath, "r+b");
    // 开始下载
    bool finish = false;
    while (!finish) {
        // 找未完成块并占有
        while (part == nullptr) {
            finish = true;
            for (auto &cur : manager->parts) {
                if (cur.progress != cur.size) {
                    finish = false;
                    // 发现未完成块，检查占有情况
                    {
                        std::lock_guard<std::mutex> guard(manager->partsLock);
                        if (manager->usingOffsets.count(cur.offset) == 0) {
                            // 未被占用则选定此块
                            manager->usingOffsets.insert(cur.offset);
                            part = &cur;
                            break;
                        }
                    }
                }
            }
            // 如果完成下载，就直接结束线程
            if (finish) {
                return;
            }
        }
        LOG(INFO) << "Select file part thread = " << getName() << ", offset = " << part->offset;
        // 建立连接
        if (!connSocket) {
            connSocket = Socket::connect(Client::INSTANCE->srvAddress, Client::INSTANCE->srvPort);
        }
        // 下载
        downloadPart();
        // 释放占有块
        {
            std::lock_guard<std::mutex> guard(manager->partsLock);
            manager->usingOffsets.erase(part->offset);
            part = nullptr;
        }
        // 检查下载是否完成
        finish = true;
        for (auto &cur : manager->parts) {
            if (cur.progress != cur.size) {
                finish = false;
                break;
            }
        }
    }
}

void DownloadThread::downloadPart() {
    if (part == nullptr) return;
    auto rest = part->size - part->progress;
    // 发送下载请求
    DownloadRequestPacket req{
            .offset = part->offset + part->progress,
            .size = rest,
    };
    ByteBuffer data(&req, sizeof(req));
    data = data + ByteBuffer::str(manager->srcPath);
    bool ret = ProtocolHelper::sendRequest(connSocket, DOWNLOAD, data);
    if (!ret) {
        return;
    }
    // 解析返回包（基本上接受就可以了）
    auto response = ProtocolHelper::receiveResponse(connSocket, *readBuffer);
    if (!response) {
        // EOF，释放连接
        connSocket = nullptr;
        return;
    }
    DownloadInfoPacket resp = *reinterpret_cast<DownloadInfoPacket const *>(response->data.data());
    if (resp.size != req.size) {
        // 大小不一致，释放连接
        LOG(ERROR) << "Download data response size mismatched, request = " << req.size << ", got = " << resp.size;
        connSocket = nullptr;
        return;
    }
    // 接受文件
    ssize_t receive, written;
    destFile->seek(part->offset + part->progress);
    while (rest > 0) {
        // 读入数据
        receive = connSocket->read(*readBuffer, std::min(readBuffer->size(), rest));
        if (receive < 0) {
            // 连接关闭
            LOG(ERROR) << "Connection closed while download";
            connSocket = nullptr;
            break;
        }
        // 写入文件
        written = destFile->write(*readBuffer, receive);
        if (receive != written) {
            // 写入错误
            PLOG(FATAL) << "Write count mismatch receive = " << receive << ", written = " << written;
            exit(1);
        }
        // 减去剩余
        rest -= receive;
        // 更新进度
        part->progress = part->size - rest;
        manager->totalProgress += written;
        manager->updateDownloadStatus(part);
    }
    LOG(INFO) << "Part downloaded: offset = " << part->offset << ", size = " << part->size;
}

DownloadThread::DownloadThread(DownloadManager *manager) : manager(manager) {}

PartStatus::PartStatus(uint64_t offset, uint64_t progress, uint64_t size) : offset(offset), progress(progress),
                                                                            size(size) {}
