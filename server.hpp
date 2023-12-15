#pragma once
#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Client.hpp"
#include "Location.hpp"
#include <poll.h>
#include "Colors.hpp"

class Server
{
	private:
		unsigned int				port;
		std::string					host;
		std::vector<Location>		locations;
		int							socket;
		std::vector<Client>			clients;

	public:
		Server();
		~Server();

		Server(unsigned int port);
		unsigned int getPort() const;
		unsigned int getSocket() const;
		std::vector<Client>& getClients();

		void	setPort(unsigned int port);
		void	setSocket(unsigned int socket);

		void	addClient();
		bool	processFd(std::vector<struct pollfd> &pollfds, struct pollfd *pollfd, int event);
		bool	isClient(std::vector<struct pollfd> &pollfds, struct pollfd *pollfd, int event);
		void	logClients();
		void	removeClient(std::vector<struct pollfd> &pollfds, std::vector<Client>::iterator &it);
};

Server::Server()
{
}

Server::Server(unsigned int port)
{
	this->port = port;
}

Server::~Server()
{
}

unsigned int Server::getPort() const {
	return this->port;
}

std::vector<Client>& Server::getClients() {
	return this->clients;
}

void	Server::setPort(unsigned int port) {
	this->port = port;
}

void	Server::setSocket(unsigned int socket) {
	this->socket = socket;
}

void	Server::addClient() {
	/* 
		title : accept new connection
	*/
	struct sockaddr_in clientAddress;
	socklen_t s_size = sizeof(clientAddress);

	int clientSocket = accept(this->socket, (struct sockaddr *)&clientAddress, &s_size);
	if (clientSocket == -1)
	    perror("accept");
	Client newClient(clientSocket, clientAddress);
	this->clients.push_back(newClient);
	std::cout << GREEN << "server active in port: " << this->port << " accepted new request" << RESET << std::endl;
}

void	Server::removeClient(std::vector<struct pollfd> &pollfds, std::vector<Client>::iterator &it)
{
	std::vector<struct pollfd>::iterator it2;

	for (it2 = pollfds.begin(); it2 != pollfds.end(); it2++) {
		if (it2->fd == it->getFd()) {
			pollfds.erase(it2);
			// nfds--
			break;
		}
	}
	this->clients.erase(it);
	std::cout << RED << "Client with fd: " << it->getFd() << " disconnected" << RESET << std::endl;
}

bool	Server::isClient(std::vector<struct pollfd> &pollfds, struct pollfd *pollfd, int event) {
	std::vector<Client>::iterator it;

	for (it = this->clients.begin(); it != this->clients.end(); it++) {
		if (it->getFd() == pollfd->fd) {
			if (event == POLLIN)
				it->readRequest(pollfd);
			else if (event == POLLOUT)
				it->sendResponse();
			else if (event == POLLHUP)
				this->removeClient(pollfds, it);
			return true;
		}
	}
	return false;
}

bool Server::processFd(std::vector<struct pollfd> &pollfds, struct pollfd *pollfd, int event) {
	/* 
		title : process file descriptor and perform operations
	*/
	/*
		if (fd == this->socket) {
			then : a new connection request
			-> this->addClient();
			fd has processed -> return true
		}
		else if (search for fd in this server) {
			then : already a connected client
			if (event == POLLIN)
				-> perform read action
			else if (event == POLLOUT)
				-> perform write action
			else if (event == POLLHUB)
				-> delete client
			fd has processed -> return true
		}
		else {
			then : fd is not related to this server
			-> do nothing
		}
		fd hasn't processed -> return false
	*/
	std::string val;
	if (event == POLLIN)
		val = "POLLIN";
	else if (event == POLLOUT)
		val = "POLLOUT";
	else if (event == POLLHUP)
		val = "POLLHUP";
	std::cout << BLUE << "-> event: " << val << " occured in fd: " << pollfd->fd << RESET << std::endl;
	if (pollfd->fd == this->socket) {
		this->addClient();
		return true;
	}
	else if (this->isClient(pollfds, pollfd, event))
	{
		return true;
	}
	else {

	}
		// do nothing
	// std::cout << "in processFd(), server with socket: " << this->socket << ", fd: " << fd << std::endl;
	return false;
}

// title : log methods

void	Server::logClients() {
	std::vector<Client>::iterator it;
	std::cout << "server at port: " << YELLOW << this->port << RESET
	<< " has " << YELLOW << this->clients.size() << RESET << " clients: " << std::endl;
	for (it = this->clients.begin(); it != this->clients.end(); it++)
		it->log();
}