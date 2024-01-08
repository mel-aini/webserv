#include "Server.hpp"

Server::Server() {}

Server::Server(std::string port, std::string host) : port(port), host(host) {}

Server::~Server()
{
	// std::cout << BOLDRED << "Server Destructor Called" << RESET << std::endl;
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

void	Server::addClient(std::vector<struct pollfd> &pollfds, nfds_t& nfds) {
	/* 
		title : accept new connection
	*/
	struct sockaddr_in clientAddress;
	socklen_t s_size = sizeof(clientAddress);

	try {
		int clientSocket = accept(this->socket, (struct sockaddr *)&clientAddress, &s_size);
		if (clientSocket == -1) {
			std::cout << "clientSocket: " << clientSocket << std::endl;
			throw ClientFailed();
		}
		int flags = fcntl(clientSocket, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
		if (flags == -1) {
			std::cout << "fcntl flags: " << flags << std::endl;
			close(clientSocket);
			throw ClientFailed();
		}
		Client newClient(clientSocket, clientAddress);
		newClient.setServerInfo(this->port, this->host, this->serverName);
		this->clients.push_back(newClient);
		std::cout << GREEN << "server active in port: " << this->port << " accepted new request" << RESET << std::endl;

		// -> monitor new fd
		struct pollfd fd;
		fd.fd = clientSocket;
		fd.events = POLLIN | POLLHUP;
		fd.revents = 0;
		std::cout << "new Client fd: " << fd.fd << std::endl;
		pollfds.push_back(fd);
		nfds++;
	}
	catch (std::exception &e) {
		std::cout << RED << e.what() << RESET << std::endl;
	}
}

void	Server::removeClient(std::vector<struct pollfd> &pollfds, nfds_t& nfds, std::vector<Client>::iterator &it)
{
	std::vector<struct pollfd>::iterator it2;

	for (it2 = pollfds.begin(); it2 != pollfds.end(); it2++) {
		if (it2->fd == it->getFd()) {
			// close file descriptor related to this client
			close(it->getFd());
			// erase it from pollfds vector
			pollfds.erase(it2);
			nfds--;
			std::cout << RED << "Client with fd: " << it2->fd << " disconnected" << RESET << std::endl;
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

bool	Server::hostsMatch(std::vector<Client>::iterator& it)
{
	// std::cout << CYAN << this->serverName << " | " << it->getRequest().getHeader("host") << RESET << std::endl;
	if (this->serverName == it->getRequest().getHeader("host"))
		return true;
	return false;
}

void	Server::transferClient(std::vector<Client>::iterator& it)
{
	this->clients.push_back(*it);
}

void	Server::findRelatedHost(std::vector<Client>::iterator& it)
{
	std::vector<Server>::iterator server = this->getServersBegin();
	std::vector<Server>::iterator end = this->getServersEnd();

	for (; server != end; server++) {
		if (this->getPort() == server->getPort() && this->getHost() == server->getHost()) {
			if (server->hostsMatch(it)) {
				server->transferClient(it);
				size_t index = std::distance(this->clients.begin(), it);
				this->clients.erase(this->clients.begin() + index);
				break;
			}
		}
	}
}

bool Server::processFd(std::vector<struct pollfd> &pollfds, struct pollfd *pollfd, nfds_t& nfds) {
	std::string val;
	bool eventOccured = false;

	std::vector<Client>::iterator it;
	if (pollfd->revents & POLLIN || pollfd->revents & POLLOUT || pollfd->revents & POLLHUP)
		eventOccured = true;
	
	if (eventOccured && pollfd->fd == this->socket) {
		this->addClient(pollfds, nfds);
		return true;
	}
	else if (this->isClient(pollfd, it))
	{
		try
		{
			if (!eventOccured) {
				// then: no event occured
				// std::cout << BLUE << "no event occured" << RESET << std::endl;
				if (it->checkLogTime()) {
					this->removeClient(pollfds, nfds, it);
					return true;
				}
				return false;
			}
			if (pollfd->revents & POLLIN) {
				// std::cout << CYAN << "#### POLLIN EVENT ####" << RESET << std::endl;
				bool read_complete = it->readRequest(this->locations, pollfd);
				//	todo: transfer client to the right server or keep it
				if (read_complete && !this->hostsMatch(it))
					this->findRelatedHost(it);
			}
			else if ((pollfd->revents & POLLOUT)) {
				// std::cout << YELLOW << "#### POLLOUT EVENT ####" << RESET << std::endl;
				bool send_complete = it->createResponse();
				if (send_complete) {
					if (it->getRequest().getHeader("connection") != "keep-alive") {
						this->removeClient(pollfds, nfds, it);
					}
					else {
						it->resHasSent();
					}
				}
			}
			else if (pollfd->revents & POLLHUP) {
				// std::cout << RED << "#### POLLHUP EVENT ####" << RESET << std::endl;
				this->removeClient(pollfds, nfds, it);
			}
		}
		catch (const std::exception& e) {
			// then: an error occured in read or send...
			this->removeClient(pollfds, nfds, it);
			std::cout << RED << e.what() << RESET << std::endl;
		}
		return true;
	}
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

std::string	Server::eraseSlash(std::string path)
{
	std::string res = path;

	if (res != "/")
	{
		if (res[res.length() - 1] == '/')
			res.erase(res.length() - 1);
		if (res[0] == '/')
			res.erase(0, 1);
	}
	return (res);
}

void	Server::setLocations(Location location, std::string num)
{
	std::vector<Location>::iterator	it = this->locations.begin();
	std::string path1;
	std::string path2;

	for (; it != locations.end(); it++)
	{
		path1 = eraseSlash(location.getPath());
		path2 = eraseSlash(it->getPath());
		if (path1 == path2)
		{
			std::cerr << "server " + num + ":" + " duplicated location" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	this->locations.push_back(location);
}

std::vector<Location>	&Server::getLocations(void)
{
	return (this->locations);
}

std::string	&Server::getPort(void)
{
	return (this->port);
}

std::string	&Server::getHost(void)
{
	return (this->host);
}

std::string	&Server::getServerName(void)
{
	return (this->serverName);
}

std::vector<Server>::iterator	&Server::getServersBegin() {
	return (this->serversBegin);
}

std::vector<Server>::iterator	&Server::getServersEnd() {
	return (this->serversEnd);
}

void	Server::setServersBegin(std::vector<Server>::iterator it) {
	this->serversBegin = it;
}

void	Server::setServersEnd(std::vector<Server>::iterator it) {
	this->serversEnd = it;
}

// title: exceptions

const char	*Server::ClientFailed::what() const throw() {
	return "Can't accept this client connection";
}
