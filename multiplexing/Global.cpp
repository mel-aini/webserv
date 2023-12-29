#include "Global.hpp"

Global::Global() : nfds(0)
{
}

Global::~Global()
{
}

void	Global::addServer(Server& server)
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

void	Global::setServers(std::vector<Server> servers) {
	this->servers = servers;
}

void	Global::monitorFd(struct pollfd fd) {
	this->pollfds.push_back(fd);
	this->nfds++;
}

void	Global::forgetFd(int fd) {
	std::vector<struct pollfd>::iterator it;

	for (it = this->pollfds.begin(); it != this->pollfds.end(); it++) {
		if (it->fd == fd) {
			this->pollfds.erase(it);
			this->nfds--;
			break;
		}
	}
}

int	Global::isAlreadyUsed(std::string host, std::string port, int index)
{
	std::vector<Server>::iterator it;
	std::vector<Server>::iterator begin;
	
	begin = this->servers.begin();
	for (it = begin; it != this->servers.end() && it - begin < index; it++) {
		if (it->getPort() == port) {
			if (it->getHost() == host)
				return it->getSocket();
			else if (it->getHost() == "localhost" && host == "127.0.0.1")
				return it->getSocket();
			else if (it->getHost() == "127.0.0.1" && host == "localhost")
				return it->getSocket();
		}
	}
	return -1;
}

void Global::checkAndProcessFd(struct pollfd *pollfds, int fds) {
	int	processed_fds = 0;

	for (unsigned int i = 0; i < this->getNfds(); i++) {
		if ((pollfds + i)->fd >= 0) {
			std::vector<Server>::iterator it;
			for (it = this->servers.begin(); it != this->servers.end(); it++) {
				if (it->processFd(this->pollfds, (pollfds + i), this->nfds))
				{
					processed_fds++;
					if (i >= this->getNfds() || processed_fds >= fds)
						return ;
					break;
				}
			}
		}
	}
}

void Global::create_servers()
{
    std::vector<Server> &servers = this->servers;
    std::vector<Server>::iterator it;

    for (it = servers.begin(); servers.size() != 0 && it != servers.end(); it++)
    {
		/*
			title: handle servers with same host and port

			int sockfd = isAddress"host:port" already in use
			if (sockfd > 0) {
				then: address already used
				-> set socket to this server
				it->setSocket(sockfd);
				continue;
			}
		*/
        int sockfd = this->isAlreadyUsed(it->getHost(), it->getPort(), it - servers.begin());
		if (sockfd > 0) {
			it->setSocket(sockfd);
			std::cout << "a server is listening on Port: " << YELLOW << it->getPort() << RESET << std::endl;
			continue;
		}
		struct addrinfo hints, *res;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(it->getHost().c_str(), it->getPort().c_str(), &hints, &res) != 0) {
            perror("getaddrinfo");
			std::cout << RED << "here" << RESET << std::endl;
            it = servers.erase(it);
			if (it == servers.end())
				break;
			continue;
        }
        if (!res) {
            it = servers.erase(it);
            std::cout << "getaddrinfo: failed" << std::endl;
			if (it == servers.end())
				break;
			continue;
        }

        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd == -1) {
            perror("sockfd");
            it = servers.erase(it);
            if (it == servers.end())
				break;
			continue;
        }
        fcntl(sockfd, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
        it->setSocket(sockfd);
    
        int optval = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
            perror("setsockopt");
            close(sockfd);
            it = servers.erase(it);
            if (it == servers.end())
				break;
			continue;
        }

        if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
            perror("bind");
            close(sockfd);
            it = servers.erase(it);
            if (it == servers.end())
				break;
			continue;
        }

        freeaddrinfo(res);
        
        if (listen(sockfd, 1) == -1) {
            perror("listen");
            close(sockfd);
            it = servers.erase(it);
            if (it == servers.end())
				break;
			continue;
        }

        // setup struct pollfd for each socket

        struct pollfd fd;
        fd.fd = sockfd;
        fd.events = POLLIN;
        fd.revents = 0;

        this->monitorFd(fd);

        std::cout << "a server is listening on Port: " << YELLOW << it->getPort() << RESET << std::endl;
    }
    if (servers.size() == 0) {
        std::cerr << "No Server has created" << std::endl;
        std::exit(EXIT_FAILURE);
    }
}