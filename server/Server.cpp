#include "Server.hpp"

Server::Server() {}

Server::Server(std::string port, std::string host) : port(port), host(host) {}

Server::~Server() {}

// title: getters

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

// title: setters

void	Server::setPort(std::string port) {
	this->port = port;
}

void	Server::setSocket(int socket) {
	this->socket = socket;
}

void	Server::setListen(std::pair<std::string, std::string> listen)
{
	if (!listen.first.empty())
		this->port = listen.first;
	if (!listen.second.empty())
		this->host = listen.second;
}

void	Server::setServerName(std::string serverName)
{
	this->serverName = serverName;
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

void	Server::setServersBegin(std::vector<Server>::iterator it) {
	this->serversBegin = it;
}

void	Server::setServersEnd(std::vector<Server>::iterator it) {
	this->serversEnd = it;
}

// title: methods

void	Server::addClient(std::vector<struct pollfd> &pollfds) {
	// title : accept new connection
	struct sockaddr_in clientAddress;
	socklen_t s_size = sizeof(clientAddress);

	try {
		int clientSocket = accept(this->socket, (struct sockaddr *)&clientAddress, &s_size);
		if (clientSocket == -1) {
			// std::cout << RED << "[ERROR] : CAN'T ACCEPT CONNECTION" << RESET << std::endl;
			throw ClientFailed();
		}
		
		if (fcntl(clientSocket, F_SETFL, O_NONBLOCK, FD_CLOEXEC) == -1) {
			close(clientSocket);
			throw ClientFailed();
		}
		std::cout << BOLDYELLOW << "[INFO] : NEW CONNECTION" << RESET << std::endl;
		Client newClient(clientSocket, clientAddress);
		newClient.setServerInfo(this->port, this->host, this->serverName);
		this->clients.push_back(newClient);

		// -> monitor new fd
		struct pollfd fd;
		fd.fd = clientSocket;
		fd.events = POLLIN | POLLHUP;
		fd.revents = 0;
	
		pollfds.push_back(fd);
	}
	catch (std::exception &e) {
		std::cout << BOLDRED << "[ERROR] : " << e.what() << RESET << std::endl;
	}
}

void	Server::removeClient(std::vector<struct pollfd> &pollfds, std::vector<Client>::iterator &it)
{
	std::vector<struct pollfd>::iterator it2;

	for (it2 = pollfds.begin(); it2 != pollfds.end(); it2++) {
		if (it2->fd == it->getFd()) {
			close(it2->fd);
			pollfds.erase(it2);
			break;
		}
	}
	this->clients.erase(it);
	std::cout << BOLDYELLOW << "[INFO] : " << BOLDRED << "CLIENT DISCONNECTED" << RESET << std::endl;
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
	return this->serverName == it->getRequest().getHeader("host");
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

bool Server::processFd(std::vector<struct pollfd> &pollfds, struct pollfd *pollfd) {
	std::vector<Client>::iterator it;

	if (pollfd->revents && pollfd->fd == this->socket) {
		this->addClient(pollfds);
		return true;
	}
	else if (this->isClient(pollfd, it))
	{
		it->setPollfd(pollfd);
		try {
			if (pollfd->revents & POLLIN) {
				bool read_complete = it->readRequest(this->locations);
				if (read_complete && !this->hostsMatch(it))
					this->findRelatedHost(it);
			}
			else if (pollfd->revents & POLLOUT) {
				bool send_complete = it->createResponse();
				if (send_complete) {
					// std::cout << YELLOW << "[INFO] : connection: " << it->getRequest().getHeader("connection") << RESET << std::endl;
					if (it->getRequest().getHeader("connection") != "keep-alive") {
						// std::cout << YELLOW << "[INFO] : CLIENT REMOVED" << RESET << std::endl;
						this->removeClient(pollfds, it);
					} else {
						it->resHasSent();
					}
				}
			}
			else if (pollfd->revents & POLLHUP) {
				this->removeClient(pollfds, it);
			}
			else {
				if (it->checkLogTime())
					this->removeClient(pollfds, it);
			}
		}
		catch (const std::exception& e) {
			this->removeClient(pollfds, it);
			std::cout << RED << e.what() << RESET << std::endl;
		}
		return true;
	}
	return false;
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

// title: exceptions

const char	*Server::ClientFailed::what() const throw() {
	return "Can't accept this client connection";
}
