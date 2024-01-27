#include "Global.hpp"

Global::Global() : nfds(0) {}

Global::~Global() {}

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

void	Global::setServers(std::vector<Server> servers) {
	this->servers = servers;
}

void	Global::monitorFd(struct pollfd fd) {
	this->pollfds.push_back(fd);
	this->nfds++;
}

int	Global::isAlreadyUsed(std::string host, std::string port, int index)
{
	std::vector<Server>::iterator it;
	std::vector<Server>::iterator begin;
	int retSocket = -1;
	begin = this->servers.begin();
	for (it = begin; it != this->servers.end() && it - begin < index; it++) {
		if (it->getPort() == port) {
			if (it->getHost() == host)
				retSocket = it->getSocket();
			else if (it->getHost() == "localhost" && host == "127.0.0.1")
				retSocket = it->getSocket();
			else if (it->getHost() == "127.0.0.1" && host == "localhost")
				retSocket = it->getSocket();
			break;
		}
	}
	std::vector<struct pollfd>::iterator it2 = this->pollfds.begin();
	for (; it2 != this->pollfds.end(); it2++) {
		if (it2->fd == retSocket) {
			return retSocket;
		}
	}
	return -1;
}

void Global::checkAndProcessFd() {
	for (unsigned int i = 0; i < getPollfds().size(); i++) {
		std::vector<Server>::iterator it;

		for (it = this->servers.begin(); it != this->servers.end(); it++) {
			if (it->processFd(this->pollfds, &getPollfds().data()[i]))
				break;
		}
	}
}

void Global::create_servers()
{
    std::vector<Server> &servers = this->servers;
    std::vector<Server>::iterator it;

    for (it = servers.begin(); servers.size() != 0 && it != servers.end(); it++)
    {
        int sockfd = this->isAlreadyUsed(it->getHost(), it->getPort(), it - servers.begin());
		if (sockfd > 0) {
			it->setSocket(sockfd);
			std::cout << "a server is listening on: " << YELLOW << it->getHost() + ":" + it->getPort() << RESET << std::endl;

			// assign begin and end iterators of servers to each server
			it->setServersBegin(this->servers.begin());
			it->setServersEnd(this->servers.end());
			continue;
		}
		struct addrinfo hints, *res;

        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(it->getHost().c_str(), it->getPort().c_str(), &hints, &res) != 0) {
            perror("getaddrinfo");
            it = servers.erase(it);
			if (it == servers.end())
				break;
			continue;
        }
        if (!res) {
            it = servers.erase(it);
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
		
        if (fcntl(sockfd, F_SETFL, O_NONBLOCK, FD_CLOEXEC) == -1) {
			perror("fcntl");
			close(sockfd);
            it = servers.erase(it);
            if (it == servers.end())
				break;
			continue;
		}

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
        
        if (listen(sockfd, SOMAXCONN) == -1) {
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

		// assign begin and end iterators of servers to each server
		it->setServersBegin(this->servers.begin());
		it->setServersEnd(this->servers.end());

        std::cout << "a server is listening on: " << YELLOW << it->getHost() + ":" + it->getPort() << RESET << std::endl;
    }
    if (pollfds.size() == 0) {
        std::cerr << "No Server has created" << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void handleSignal(int signal) {
    if (signal == SIGPIPE) {
        std::cerr << BOLDRED << "[ERROR] : SIGPIPE CAUGHT" << RESET << std::endl;
    }
}