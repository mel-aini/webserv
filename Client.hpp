#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include "Colors.hpp"
#include <poll.h>
#include <sstream>
#include <cstring>
#include <string>
#include <fstream>
#include "Request.hpp"
#include "Response.hpp"

enum read_state {
	REQUEST_LINE,
	HEADERS,
	BODY,
	EOR,
};

class Client
{
	private:
		int					fd;
		struct sockaddr_in	address;
		bool				resIsSent;
		bool				reqIsArrived;
		bool				firstInteraction;
		struct pollfd		*pollfd;
		Request				request;
		Response			response;


	public:
		Client(int fd, struct sockaddr_in address);
		~Client();

		int			getFd() const;
		struct sockaddr_in getAddress() const;
		void		log();
		void		readRequest(struct pollfd *pollfd);
		void		sendResponse(std::string host);
		void		reqHasRead();
		void		resHasSent();
		void		reset();
		void		setPollfd(struct pollfd	*pollfd);
		void		getMethod();
		void		postMethod();
		void		deleteMethod();
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
	if (this->firstInteraction) {
		setPollfd(pollfd);
		this->firstInteraction = false;
	}
	// char buf[2];
	char buf[1024] = {0};
	int readed = recv(this->fd, buf, 1, 0);
	if (readed == -1 || readed == 0) {
		// this->request.resetBuffer();
		this->reqHasRead();
		// exit(1);
		// then: close connection
		// this->reqHasRead();
	}
	/*
		read 1024, still 1000
		read 1000, 
	*/
	// std::cout << buf << std::endl;
	this->request.appendToBuffer(buf);
	std::cout << "http request message: " << std::endl;
	std::cout << buf;
	// this->request.resetBuffer();
	this->reqHasRead();
}

void		Client::sendResponse(std::string host) {
	/*
		if (method is GET)

	*/
	// std::cout << "is Sent: " << this->resIsSent << std::endl;
	// std::cout << "is Arrived: " << this->reqIsArrived << std::endl;
	if (this->reqIsArrived) {
		std::string res = "HTTP/1.1 200 OK\n";
		std::string fileName;
		if (host == "host1")
			fileName = "html/index.html";
		else if (host == "host2")
			fileName = "html/index2.html";
		else if (host == "host3")
			fileName = "html/index3.html";
		std::ifstream file(fileName, std::ios::in);
		if (!file.is_open()) {
			std::cerr << BOLDRED << "Error: Unable to open infile" << RESET << std::endl;
            return ;
		}
		std::stringstream sstream;
		sstream << file.rdbuf();
		std::string body = sstream.str();
		file.close();
		// std::string body = "<html>\
		// <head><title>404 Not Found</title></head>\
		// <body>\
		// <center><h1>404 Not Found</h1></center>\
		// <hr><center>webserv</center>\
		// </body>\
		// </html>";

		int len = body.size();
		std::ostringstream s;
		s << len;
		std::string contentLenght = "Keep-Alive: timeout=2, max=1000\nContent-Length: " + s.str() + "\n";
		res += contentLenght;
		std::string contentType = "Content-Type: text/html; charset=utf-8\r\n\r\n";
		res += contentType;
		res += body;
		// std::cout << RED << res << RESET << std::endl; 
		const char *buf = res.c_str();
		send(this->fd, buf, res.size(), 0);
		this->resIsSent = true;
		this->resHasSent();
	}
}

void	Client::reqHasRead()
{
	this->reqIsArrived = true;
	std::cout << MAGENTA << "reqHasRead()" << RESET << std::endl;
	this->pollfd->events = POLLIN | POLLOUT | POLLHUP;
}

void	Client::resHasSent()
{
	std::cout << YELLOW << "resHasSent()" << RESET << std::endl;
	this->pollfd->events = POLLIN | POLLHUP;
}

void	Client::reset()
{
	std::cout << "reset()" << std::endl;
	this->pollfd->events = POLLIN | POLLHUP;
}