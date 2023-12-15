#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include "Colors.hpp"
#include <poll.h>
#include <sstream>
#include <cstring>
#include <string>

class Client
{
	private:
		int					fd;
		struct sockaddr_in	address;
		bool				resIsSent;
		bool				reqIsArrived;
		bool				firstInteraction;
		struct pollfd		*pollfd;

	public:
		Client(int fd, struct sockaddr_in address);
		~Client();

		int getFd() const;
		struct sockaddr_in getAddress() const;
		void		log();
		void		readRequest(struct pollfd *pollfd);
		void		sendResponse();
		void		reqHasRead();
		void		resHasSent();
		void		reset();
		void		setPollfd(struct pollfd	*pollfd);
};

Client::Client(int fd, struct sockaddr_in address)
	:
	fd(fd),
	address(address),
	resIsSent(false),
	firstInteraction(true),
	pollfd(NULL) {}

Client::~Client() {}

int Client::getFd() const {
	return this->fd;
}
struct sockaddr_in Client::getAddress() const {
	return this->address;
}

// title : log methods

void	Client::setPollfd(struct pollfd	*pollfd) {
	this->pollfd = pollfd;
}

void Client::log() {
	std::cout << "client with fd: " << this->fd << std::endl;
}

void		Client::readRequest(struct pollfd *pollfd) {
	if (this->firstInteraction)
		setPollfd(pollfd);
	char buf[1000] = {0};
	if (recv(this->fd, buf, sizeof(buf), 0) == -1) {
		perror("recv");
	}
	this->resIsSent = false;
	std::cout << "http request message: " << std::endl;
	std::cout << buf;
	this->reqHasRead();
}

void		Client::sendResponse() {
	if (this->resIsSent == false && reqIsArrived) {
		std::string res = "HTTP/1.1 200 OK\n";
		std::string body = "<html>\
		<head><title>404 Not Found</title></head>\
		<body>\
		<center><h1>404 Not Found</h1></center>\
		<hr><center>webserv</center>\
		</body>\
		</html>";
		int len = body.size();
		std::ostringstream s;
		s << len;
		std::string contentLenght = "Content-Length: " + s.str() + "\n";
		res += contentLenght;
		std::string contentType = "Content-Type: text/html; charset=utf-8\r\n\r\n";
		res += contentType;
		res += body;
		std::cout << RED << res << RESET << std::endl; 
		const char *buf = res.c_str();
		send(this->fd, buf, res.size(), 0);
		this->resIsSent = true;
		this->resHasSent();
	}
}

void	Client::reqHasRead()
{
	this->pollfd->events = POLLIN | POLLOUT | POLLHUP;
}

void	Client::resHasSent()
{
	this->pollfd->events = POLLIN | POLLHUP;
}

void	Client::reset()
{
	this->pollfd->events = POLLIN | POLLHUP;
}