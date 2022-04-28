
#include "iomanager.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstring>

int sockfd;
void watch_io_read();

// 写事件回调，只执行一次，用于判断非阻塞套接字connect成功
void do_io_write() {
    P0ROOT_LOG_INFO() << "write callback";
    int so_err;
    socklen_t len = size_t(so_err);
    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_err, &len);
    if(so_err) {
        P0ROOT_LOG_INFO() << "connect fail";
        return;
    } 
    P0ROOT_LOG_INFO() << "connect success";
}

// 读事件回调，每次读取之后如果套接字未关闭，需要重新添加
void do_io_read() {
    P0ROOT_LOG_INFO() << "read callback";
    char buf[1024] = {0};
    int readlen = 0;
    readlen = read(sockfd, buf, sizeof(buf));
    if(readlen > 0) {
        buf[readlen] = '\0';
        P0ROOT_LOG_INFO() << "read " << readlen << " bytes, read: " << buf;
    } else if(readlen == 0) {
        P0ROOT_LOG_INFO() << "peer closed";
        close(sockfd);
        return;
    } else {
        P0ROOT_LOG_ERROR() << "err, errno=" << errno << ", errstr=" << strerror(errno);
        close(sockfd);
        return;
    }
    // read之后重新添加读事件回调，这里不能直接调用addEvent，因为在当前位置fd的读事件上下文还是有效的，直接调用addEvent相当于重复添加相同事件
    phase0::IOManager::GetThis()->schedule(watch_io_read);
}

void watch_io_read() {
    P0ROOT_LOG_INFO() << "watch_io_read";
    phase0::IOManager::GetThis()->addEvent(sockfd, phase0::IOManager::READ, do_io_read);
}

void test_io() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    PHASE0_ASSERT(sockfd > 0);
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(10002);
    inet_pton(AF_INET, "120.76.100.197", &servaddr.sin_addr.s_addr);

    int rt = connect(sockfd, (const sockaddr*)&servaddr, sizeof(servaddr));
    if(rt != 0) {
        if(errno == EINPROGRESS) {
            P0ROOT_LOG_INFO() << "EINPROGRESS";
            // 注册写事件回调，只用于判断connect是否成功
            // 非阻塞的TCP套接字connect一般无法立即建立连接，要通过套接字可写来判断connect是否已经成功
            phase0::IOManager::GetThis()->addEvent(sockfd, phase0::IOManager::WRITE, do_io_write);
            // 注册读事件回调，注意事件是一次性的
            phase0::IOManager::GetThis()->addEvent(sockfd, phase0::IOManager::READ, do_io_read);
        } else {
            P0ROOT_LOG_ERROR() << "connect error, errno:" << errno << ", errstr:" << strerror(errno);
        }
    } else {
        P0ROOT_LOG_ERROR() << "else, errno:" << errno << ", errstr:" << strerror(errno);
    }
}

void test_iomanager() {
    // phase0::IOManager iom;
    phase0::IOManager iom(3); 
    iom.schedule(test_io);
}

int main(int argc, char *argv[]) {
    test_iomanager();

    return 0;
}