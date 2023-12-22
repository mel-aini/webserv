#pragma once
#include <iostream>
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

		nfds_t	getNfds();

		void addServer(Server& server);

		void monitorFd(struct pollfd fd);
		void forgetFd(int fd);
		void setServers(std::vector<Server> servers);
		void checkAndProcessFd(struct pollfd *pollfd);
};

