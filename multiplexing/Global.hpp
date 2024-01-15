#pragma once
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include "../public/Colors.hpp"
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include "../server/Server.hpp"

class Global
{
	private:
		std::vector<Server>			servers;
		std::vector<struct pollfd>	pollfds;
		nfds_t						nfds;

	public:
		Global();
		~Global();

		std::vector<Server>& getServers();

		std::vector<struct pollfd>& getPollfds();

		void addServer(Server& server);

		void	create_servers();
		void	monitorFd(struct pollfd fd);
		void	setServers(std::vector<Server> servers);
		void	checkAndProcessFd();
		int		isAlreadyUsed(std::string host, std::string port, int index);
};

