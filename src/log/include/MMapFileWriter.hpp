#pragma once

#include <Timestamp.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <string>

#include "Writer.h"

namespace phase0
{
class MMapFileWriter : public Writer
{
public:
    MMapFileWriter(std::string basename)
        : Writer(basename), fd_(-1), memSize_(1024 * 60), writen_(0), flushPos_(0), partNum(1)
    {
        createResources(getFileName());
    };
    ~MMapFileWriter() { destoryResources(); };
    void append(const char* data, int32_t len)
    {
        if (len > memSize_ - writen_)
        {
            fprintf(stderr, "mmap memory overflow ,errno=%d", errno);
            return;
        }
        std::cout << "data:" << writen_ << std::endl;
        memcpy(buffer_ + writen_, data, len);
        writen_ += len;
    }
    void flush()
    {
        if (buffer_ != MAP_FAILED)
        {
            // msync(buffer_, memSize_, MS_ASYNC);
            msync(buffer_ + flushPos_, writen_, MS_ASYNC);
            // std::cout << "data:\n" << std::string(buffer_, writen_) << std::endl;
            flushPos_ = writen_;
        }
    }
    uint32_t writtenBytes() const { return writen_; }
    void reset()
    {
        destoryResources();
        writen_ = 0;
        partNum++;
        createResources(getFileName());
    }

private:
    void createResources(std::string fileName)
    {
        if (fd_ >= 0)
        {
            close(fd_);
        }

        fileName.append(".txt");
        fd_ = open(fileName.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd_ < 0)
        {
            fprintf(stderr, "open new file failed, errno=%d", errno);
        }
        else
        {
            int n = ftruncate(fd_, memSize_);
            buffer_ = (char*)mmap(NULL, memSize_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
            if (buffer_ == MAP_FAILED)
            {
                fprintf(stderr, "mmap file failed,errno=%d", errno);
            }
        }
    }
    void destoryResources()
    {
        if (fd_ >= 0)
        {
            close(fd_);
        }
        if (buffer_ != MAP_FAILED)
        {
            munmap(buffer_, memSize_);
        }
    }
    std::string getFileName() const { return basename_ + Timestamp::now().toStringYMM() + "(" + std::to_string(partNum) + ")"; }

    int fd_;
    char* buffer_;
    int32_t memSize_;
    int32_t writen_;
    int32_t flushPos_;
    size_t partNum;
};
}  // namespace phase0