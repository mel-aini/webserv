#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include "multiplexing/Global.hpp"
#include <poll.h>
#include "public/Colors.hpp"
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include "parsing/ConfigFile.hpp"

using std::cerr;
using std::cout;
using std::endl;

enum mothods {
    GET, 
    POST,
    DELETE
};

int main(int ac, char* av[])
{
    if (ac != 2)
	{
		cerr << "Error" << endl << "invalid arguments" << endl;
		return (1);
	}
    Global global;
    // -> parsing
    global.setServers(parser(av[1]));

    // create_servers(global);
    global.create_servers();
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
            // then: no event occurs in that specified time
            /*
            std::cout << YELLOW << "no event occurs in that specified time" << RESET << std::endl;
            for (unsigned int i = 0; i < global.getNfds(); i++) {
                if ((pollfds + i)->fd > 0) {
                    std::cout << "fd: " << (pollfds + i)->fd << " monitor: ";
                    if (((pollfds + i)->events & POLLIN) == POLLIN)
                        std::cout << "POLLIN, ";
                    if (((pollfds + i)->events & POLLOUT) == POLLOUT)
                        std::cout << "POLLOUT, ";
                    if (((pollfds + i)->events & POLLHUP) == POLLHUP)
                        std::cout << "POLLHUP";
                    std::cout << std::endl;
                }
            }
            */
            continue;
        }
        try
        {
            for (unsigned int i = 0; i < global.getNfds(); i++) {
                if ((pollfds + i)->fd > 0) {
                    global.checkAndProcessFd(pollfds + i);
                }
            }
        } catch(const std::exception& e) {
            std::cerr << RED << e.what() << RESET << std::endl;
        }
    }
}