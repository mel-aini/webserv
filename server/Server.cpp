#include "Server.hpp"

Server::Server() {}

Server::Server(std::string port, std::string host) : port(port), host(host) {}

Server::~Server()
{
}

std::string Server::getPort() const {
	return this->port;
}

std::string Server::getHost() const {
	return this->host;
}

int Server::getSocket() const {
	return this->socket;
}

std::vector<Client>& Server::getClients() {
	return this->clients;
}

void	Server::setPort(std::string port) {
	this->port = port;
}

void	Server::setSocket(int socket) {
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
	fcntl(clientSocket, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
	Client newClient(clientSocket, clientAddress);
	this->clients.push_back(newClient);
	std::cout << GREEN << "server active in port: " << this->port << " accepted new request" << RESET << std::endl;
}

void	Server::removeClient(std::vector<struct pollfd> &pollfds, std::vector<Client>::iterator &it)
{
	std::vector<struct pollfd>::iterator it2;

	for (it2 = pollfds.begin(); it2 != pollfds.end(); it2++) {
		if (it2->fd == it->getFd()) {
			// close file descriptor related to this client
			std::cout << RED << "Client with fd: " << it2->fd << " disconnected" << RESET << std::endl;
			close(it->getFd());
			// erase it from pollfds vector
			pollfds.erase(it2);
			// nfds--
			break;
		}
	}
	this->clients.erase(it);
}

bool	Server::isClient(struct pollfd *pollfd, std::vector<Client>::iterator &it) {
	for (it = this->clients.begin(); it != this->clients.end(); it++) {
		if (it->getFd() == pollfd->fd)
			return true;
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
			if (event == POLLIN) {
				-> read request
				-> parse it
				-> fill members
				-> validate request
				// -> perform read action
			}
			else if (event == POLLOUT)
				-> generate response
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
	std::cout << CYAN << "-> event: " << val << " occured in fd: " << pollfd->fd << RESET << std::endl;

	std::vector<Client>::iterator it;

	if (pollfd->fd == this->socket) {
		this->addClient();
		return true;
	}
	else if (this->isClient(pollfd, it))
	{
		if (event == POLLIN) {
			it->readRequest(pollfd);
			/*
				-> parse it
					generate response if needed, ex: bad request...
				-> assign Client Request members
				-> 
			*/
		}
		else if (event == POLLOUT) {
			it->createResponse(this->host);
		}
		else if (event == POLLHUP) {
			this->removeClient(pollfds, it);
		}
		return true;
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

void	Server::setListen(std::pair<std::string, std::string> listen)
{
	this->port = listen.first;
	this->host = listen.second;
}

void	Server::setServerName(std::string serverName)
{
	this->serverName = serverName;
}

void	Server::setLocations(Location location)
{
	this->locations.push_back(location);
}

std::vector<Location>	Server::getLocations(void)
{
	return (this->locations);
}