#include "Global.hpp"

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

void Global::setServers(std::vector<Server> servers) {
	this->servers = servers;
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
						-> should add it to pollfds to monitor
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