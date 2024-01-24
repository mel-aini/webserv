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
        std::cerr << BOLDRED << "[ERROR] : SIGPIPE CAUGHT" << RESET << std::endl;
    }
}

int main(int ac, char* av[])
{
    Global global;
    global.setServers(parser(ac, av));
    global.create_servers();


    while (true)
    {
        // std::cout << "server life time: " << BOLDRED << serverLifeTime << RESET << std::endl;
        signal(SIGPIPE, handleSignal);

        int fds = poll(global.getPollfds().data(), global.getPollfds().size(), 30000);
        if (fds == -1) {
            perror("poll");
            continue;
        }
        try  {
            global.checkAndProcessFd();
        }
        catch(const std::exception& e) {
            std::cerr << RED << "in main: " << e.what() << RESET << std::endl;
        }
    }
}
