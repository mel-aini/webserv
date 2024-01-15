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

    // size_t  serverLifeTime = 0;
	// size_t  serverLifeTime_start = time(0);

    while (true)
    {
        // std::cout << "server life time: " << BOLDRED << serverLifeTime << RESET << std::endl;
        signal(SIGPIPE, handleSignal);

        int fds = poll(global.getPollfds().data(), global.getPollfds().size(), 30000);
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
            global.checkAndProcessFd();
        }
        catch(const std::exception& e) {
            std::cerr << RED << "in main: " << e.what() << RESET << std::endl;
        }
        // serverLifeTime = time(0) - serverLifeTime_start;
    }
}
