#include "socket.h"

int startServer(){
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags    = AI_PASSIVE;
    status = getaddrinfo(NULL, SERVERPORT, &host_info, &host_info_list);   
    if (status != 0) {
        throw myException("Error: cannot get address info.");
    }
    // set a socket descriptor
    socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
    if (socket_fd == -1) {
        throw myException("Error: cannot create socket.");
    } 
    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        throw myException("Error: cannot bind socket.");
    }
    // listen, accept and receive
    status = listen(socket_fd, BACKLOG);
    if (status == -1) {
        throw myException("Error: cannot listen on socket.");
    }
    freeaddrinfo(host_info_list);
    return socket_fd;
}

int acceptConnection(int socket_fd){
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int socket_fd_new;
    socket_fd_new= accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if(socket_fd_new==-1){
        throw myException("Error: cannot accept connection on socket.");
    }
    struct sockaddr_in * addr = (struct sockaddr_in *)&socket_addr;
    //std::cout << "Accepted client ip address: "<<inet_ntoa(addr->sin_addr) << std::endl;
    string clientIp = inet_ntoa(addr->sin_addr);
    return socket_fd_new;
}

int startClient(const char * hostname, const char * port){
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        throw myException("Error: cannot get address info for host.");
    }
    // set a socket descriptor
    socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
    if (socket_fd == -1) {
        throw myException("Error: cannot create socket.");
    }
    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        throw myException("Error: cannot connect to socket.");
    }
    freeaddrinfo(host_info_list);
    return socket_fd;
}
void closefd(int socket_fd){
    close(socket_fd);
}