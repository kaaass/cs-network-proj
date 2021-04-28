#include "DownloadManager.h"

#include "Client.h"

#define READ_BUFFER_PER_THREAD (64 * 1048576)
#define PART_SIZE (64 * 1048576)

void DownloadManager::init() {
    // 初始化下载缓冲区
    for (int i = 0; i < Client::INSTANCE->downloadThreads; i++) {
        readBuffers.push_back(nullptr);
    }
}

bool DownloadManager::downloadWait(const std::string &srcFile, const std::shared_ptr<File> &destFile, uint64_t size) {
    // 初始化状态
    printf("Initializing download...\n");
    srcPath = srcFile;
    destPath = destFile->getRealPath();
    totalProgress = 0;
    totalSize = size;
    parts.clear();
    usingOffsets.clear();
    // 初始化线程池
    threadPool = std::make_unique<ThreadPool<DownloadThread>>(
            "DownloadPool", Client::INSTANCE->downloadThreads, this);
    auto &threads = threadPool->lists();
    // 检查断点续传
    uint32_t nPart;
    partFile = File::open(destPath + ".dlinfo", "r+b");
    if (partFile) {
        // 继续下载
        // 读取块数
        nPart = partFile->getSize() / sizeof(PartStatus);
        LOG(INFO) << "Find part file with nPart = " << nPart;
        // 读取下载状态
        ByteBuffer partBuf;
        partBuf.allocate(sizeof(PartStatus));
        for (int i = 0; i < nPart; i++) {
            partFile->seek(i * sizeof(PartStatus));
            partFile->read(partBuf, sizeof(PartStatus));
            parts.push_back(*reinterpret_cast<PartStatus *>(partBuf.data()));
            totalProgress += parts.back().size - parts.back().rest;
        }
    } else {
        // 新下载
        // 预分配文件大小
        destFile->allocate(size);
        // 划分下载分块
        uint64_t curPos = 0;
        auto rest = size;
        nPart = 0;
        while (rest >= PART_SIZE) {
            parts.emplace_back(nPart++, curPos, PART_SIZE, PART_SIZE);
            curPos += PART_SIZE;
            rest -= PART_SIZE;
        }
        if (rest > 0) {
            parts.emplace_back(nPart++, curPos, rest, rest);
        }
        // 生成下载状态文件
        partFile = File::open(destPath + ".dlinfo", "wb");
        partFile->write(reinterpret_cast<uint8_t *>(parts.data()), nPart * sizeof(PartStatus));
    }
    usingOffsets.assign(nPart, 0);
    // 设置线程池参数：读缓冲、开始搜索位置
    for (int i = 0; i < threads.size(); i++) {
        threads[i]->threadNo = i;
        threads[i]->readBuffer = readBuffers[i];
        threads[i]->startPart = i * nPart / threads.size();
    }
    // 准备下载
    start = std::chrono::system_clock::now();
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
    partFile->remove();
    return true;
}

void DownloadManager::updateDownloadStatus(PartStatus *part) {
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    averageSpeed = double(totalProgress) / elapsed_seconds.count();
    printf("Download progress: %.2lf%%, %.2lf MB/s\n",
           double(totalProgress) / totalSize * 100,
           averageSpeed / (1024. * 1024.));
    // 状态文件
    if (partFile) {
        partFile->seek(part->id * sizeof(PartStatus));
        partFile->write(reinterpret_cast<uint8_t *>(part), sizeof(PartStatus));
    }
}

void DownloadThread::run() {
    // 初始化缓冲区
    if (!readBuffer) {
        readBuffer = std::make_shared<ByteBuffer>();
        readBuffer->allocate(READ_BUFFER_PER_THREAD);
        // 回写 Manager 以复用下载缓冲
        manager->readBuffers[threadNo] = readBuffer;
    }
    // 打开线程本地用文件
    destFile = File::open(manager->destPath, "r+b");
    // 开始下载
    bool finish;
    auto &partsLock = manager->partsLock;
    auto &usingOffsets = manager->usingOffsets;
    auto &parts = manager->parts;
    auto nParts = parts.size();
    while (true) {
        // 找未完成块并占有
        {
            std::lock_guard<std::mutex> guard(partsLock);
            while (part == nullptr) {
                finish = true;
                for (size_t i = startPart; i < nParts; i++) {
                    auto &cur = parts[i];
                    if (cur.rest) {
                        finish = false;
                        // 发现未完成块，检查占有情况
                        if (!usingOffsets[cur.id]) {
                            // 未被占用则选定此块
                            usingOffsets[cur.id] = 1;
                            part = &cur;
                            break;
                        }
                    }
                }
                // 如果完成下载，就直接结束线程
                if (finish) {
                    return;
                }
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
            std::lock_guard<std::mutex> guard(partsLock);
            usingOffsets[part->id] = 0;
        }
        // 循环更新 startPart
        startPart = (part->id + 1) % nParts;
        part = nullptr;
    }
}

void DownloadThread::downloadPart() {
    if (part == nullptr) return;
    auto rest = part->rest;
    // 发送下载请求
    DownloadRequestPacket req{
            .offset = part->offset + part->size - rest,
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
    destFile->seek(req.offset);
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
        part->rest = rest;
        manager->totalProgress += written;
        manager->updateDownloadStatus(part);
    }
    LOG(INFO) << "Part downloaded: offset = " << part->offset << ", size = " << part->size;
}

DownloadThread::DownloadThread(DownloadManager *manager) : manager(manager) {}

PartStatus::PartStatus(uint32_t id, uint64_t offset, uint64_t rest, uint64_t size) :
        id(id), offset(offset), rest(rest), size(size) {}
