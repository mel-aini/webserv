#include <iostream>
#include "multiplexing/Global.hpp"
#include <poll.h>
#include "public/Colors.hpp"
#include <sys/types.h>
#include "parsing/ConfigFile.hpp"
#include <csignal>

int main(int ac, char* av[])
{
    Global global;
    global.setServers(parser(ac, av));
    global.create_servers();


    while (true)
    {
        signal(SIGPIPE, handleSignal);

        int fds = poll(global.getPollfds().data(), global.getPollfds().size(), 60000);
        if (fds == -1) {
            perror("poll");
            continue;
        }
        try  {
            global.checkAndProcessFd();
        }
        catch(const std::exception& e) {
            std::cerr << RED << e.what() << RESET << std::endl;
        }
    }
}
