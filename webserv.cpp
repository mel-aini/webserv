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
#include <csignal>

using std::cerr;
using std::cout;
using std::endl;

void handleSignal(int signal) {
    if (signal == SIGPIPE) {
        std::cerr << BOLDRED << "Caught SIGPIPE signal" << RESET << std::endl;
        // Handle the broken pipe signal here
    }
}

int main(int ac, char* av[])
{
    Global global;
    global.setServers(parser(ac, av));
    global.create_servers();
    struct pollfd *pollfds;


    while (true)
    {
        signal(SIGPIPE, handleSignal);
        pollfds = global.getPollfds().data();

        int fds = poll(pollfds, global.getPollfds().size(), 5000);
        if (fds == -1) {
            perror("poll");
            continue;
        }
        else if (fds == 0) {
            // then: no event occurs in that specified time
            // std::cout << "no event occurs in that specified time" << std::endl;
            continue;
        }
        try  {
            global.checkAndProcessFd(fds);
        } catch(const std::exception& e) {
            std::cerr << RED << "in main: " << e.what() << RESET << std::endl;
        }
    }
}