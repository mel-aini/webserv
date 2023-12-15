#pragma once
#include <iostream>
#include "server.hpp"

class Global
{
	private:
		std::vector<Server>			servers;
		std::vector<struct pollfd>	pollfds;
		nfds_t						nfds;
		// std::vector<int> 			servers_fds;
		// std::vector<int> 			clients_fds;

	public:
		Global();
		~Global();

		std::vector<Server>& getServers();

		std::vector<struct pollfd>& getPollfds();

		nfds_t	getNfds();

		void addServer(Server& server);

		void monitorFd(struct pollfd fd);
		void forgetFd(int fd);

		void checkAndProcessFd(struct pollfd *pollfd);
};

Global::Global() : nfds(0)
{
}

Global::~Global()
{
}

void Global::addServer(Server& server)
{
	this->servers.push_back(server);
}

std::vector<Server>& Global::getServers() {
	return this->servers;
}

std::vector<struct pollfd>& Global::getPollfds() {
	return this->pollfds;
}

nfds_t	Global::getNfds() {
	return this->nfds;
}

void Global::monitorFd(struct pollfd fd) {
	this->pollfds.push_back(fd);
	this->nfds++;
}

void Global::forgetFd(int fd) {
	std::vector<struct pollfd>::iterator it;

	for (it = this->pollfds.begin(); it != this->pollfds.end(); it++) {
		if (it->fd == fd) {
			this->pollfds.erase(it);
			this->nfds--;
			break;
		}
	}
}

void Global::checkAndProcessFd(struct pollfd *pollfd) {
	std::vector<Server>::iterator it;
	if (((pollfd)->revents & POLLIN) == POLLIN) {
		for (it = this->servers.begin(); it != this->servers.end(); it++) {
			unsigned long sizeBefore = it->getClients().size();

			if (int newfd = it->processFd(this->pollfds, pollfd, POLLIN)) {
				std::vector<Client> &clients = it->getClients();
				// std::cout << "size before: " << sizeBefore << std::endl;
				// std::cout << "size after: " << clients.size() << std::endl;
				if (clients.size() > sizeBefore) {
					/*
						then : a new client added
						-> should add it to pollfds
					*/
					struct pollfd fd;
					fd.fd = clients[clients.size() - 1].getFd();
					fd.events = POLLIN | POLLHUP;
					fd.revents = 0;
					std::cout << "new Client fd: " << fd.fd << std::endl;
					this->monitorFd(fd);
				}
				break ;
			}
		}
	}
	if (((pollfd)->revents & POLLOUT) == POLLOUT) {
		for (it = this->servers.begin(); it != this->servers.end(); it++) {
			if (int newfd = it->processFd(this->pollfds, pollfd, POLLOUT)) {
				break ;
			}
		}
	}
	if (((pollfd)->revents & POLLHUP) == POLLHUP) {
		for (it = this->servers.begin(); it != this->servers.end(); it++) {
			if (int newfd = it->processFd(this->pollfds, pollfd, POLLHUP)) {
				this->nfds--;
				break ;
			}
		}
	}
}