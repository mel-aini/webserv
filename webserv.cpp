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

int main(int ac, char* av[])
{
    Global global;
    global.setServers(parser(ac, av));
    global.create_servers();
    struct pollfd *pollfds;
    while (true)
    {
        pollfds = &global.getPollfds()[0];

        int fds = poll(pollfds, global.getNfds(), 5000);
        if (fds == -1) {
            perror("poll");
            continue;
        }
        else if (fds == 0) {
            // then: no event occurs in that specified time
            continue;
        }
        try  {
            global.checkAndProcessFd(pollfds, fds);
        } catch(const std::exception& e) {
            std::cerr << RED << e.what() << RESET << std::endl;
        }
    }
}