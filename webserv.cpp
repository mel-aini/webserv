#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include "Global.hpp"
#include <poll.h>
#include "Colors.hpp"
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>

#define PORT 8000

enum mothods {
    GET, 
    POST,
    DELETE
};

void create_servers(Global& global)
{
    std::vector<Server> &servers = global.getServers();
    std::vector<Server>::iterator it;
    it = servers.begin();

    for (it = servers.begin(); servers.size() != 0 && it != servers.end(); it++)
    {
        struct addrinfo hints, *res;
        
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        std::ostringstream s;
        s << it->getPort();
        if (getaddrinfo("127.0.0.1", s.str().c_str(), &hints, &res) != 0) {
            perror("getaddrinfo");
            it = servers.erase(it);
            continue;
        }
        if (!res) {
            it = servers.erase(it);
            std::cout << "getaddrinfo: failed" << std::endl;
            continue;
        }

        int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd == -1) {
            perror("sockfd");
            it = servers.erase(it);
            continue;
        }
        fcntl(sockfd, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
        it->setSocket(sockfd);
    
        int optval = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
            perror("setsockopt");
            close(sockfd);
            it = servers.erase(it);
            continue;
        }

        if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
            perror("bind");
            close(sockfd);
            it = servers.erase(it);
            continue;
        }

        freeaddrinfo(res);
        
        if (listen(sockfd, 1) == -1) {
            perror("listen");
            close(sockfd);
            it = servers.erase(it);
            continue;
        }

        // setup struct pollfd for each socket

        struct pollfd fd;
        fd.fd = sockfd;
        fd.events = POLLIN;
        fd.revents = 0;

        global.monitorFd(fd);

        std::cout << "a server is listening on Port: " << YELLOW << it->getPort() << RESET << std::endl;
    }
    if (servers.size() == 0) {
        std::cerr << "No Server has created" << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

int main()
{
    Global global;
    // -> parsing
    Server server1(8000, "host1");
    // Server server2(8001, "host2");
    // Server server3(8002, "host3");
    global.addServer(server1);
    // global.addServer(server2);
    // global.addServer(server3);

    create_servers(global);
    struct pollfd *pollfds;

    while (true)
    {
        /*
            title : pseudo code for multiservers
        */
        /*
            -> poll()

            for (through fds : fd) {
                if ((client + fd)->fd > 0 (valid fd) {
                    if (((client + fd)->revents & POLLIN) == POLLIN)
                    {
                        then : fd is ready to read from
                        for (iterate through servers) {
                            if (server.processFd((client + fd)->fd), POLLIN) {
                                then : server has processed fd
                                break ;
                            }
                        }
                    }
                    if (((client + fd)->revents & POLLOUT) == POLLOUT)
                    {
                        then : fd is ready to write in
                        for (iterate through servers) {
                            if (server.processFd((client + fd)->fd), POLLOUT) {
                                then : server has processed fd
                                break ;
                            }
                        }
                    }
                }
            }
        */
        pollfds = &global.getPollfds()[0];

        int fd = poll(pollfds, global.getNfds(), 5000);
        if (fd == -1) {
            perror("poll");
            continue;
        }
        else if (fd == 0) {
            // std::cout << "no event occurs in that specified time" << std::endl;
            // // no event occurs in that specified time
            // std::vector<Server> &servers = global.getServers();
            // std::vector<Server>::iterator it;
            
            // for (it = servers.begin(); it != servers.end(); it++)
            //     it->logClients();
            continue;
        }

        for (unsigned int i = 0; i < global.getNfds(); i++) {
            if ((pollfds + i)->fd > 0) {
                global.checkAndProcessFd(pollfds + i);
            }
        }
    }
}