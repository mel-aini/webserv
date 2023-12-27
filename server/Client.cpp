#include "Client.hpp"

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

bool		Client::readRequest(struct pollfd *pollfd) {
	if (this->firstInteraction) {
		setPollfd(pollfd);
		this->firstInteraction = false;
	}
	// char buf[2];
	char buf[1024] = {0};
	int readed = recv(this->fd, buf, sizeof(buf), 0);
	if (readed == -1 || readed == 0) {
		// this->request.resetBuffer();
		this->reqHasRead();
		// then: close connection
	}
	/*
		read 1024, still 1000
		read 1000, 
	*/
	// std::cout << RED << buf << RESET << std::endl;
	std::cout << "readed: " << readed << std::endl;
	if (this->request.readRequest(buf, readed))
		this->reqHasRead();
	this->request.printRequest();
	// std::cout << buf;
	// this->request.resetBuffer();
	//this->reqHasRead();
	/*
		if (still reading request)
			return false;
		then: request has finished reading, return true
	*/
	return true;
}

void		Client::createResponse(std::string host) {
	/*
		-> find location that matches with uri
		-> get methods allowed, 
		if methods allowed not specified
		-> use GET POST DELETE

		if (status != 200) {
			then: a client error found in the request
			-> send 4xx response(status)
		}
		else if (location has a redirect) {
			-> perfrom redirect
		}
		else if (method is allowed) {
			if (GET)
				-> perform action, getMethod()
			else if (POST)
				-> perform action, postMethod()
			else if (DELETE)
				-> perform action, deleteMethod()
		}
		else {
			then: Method Not Allowed
			-> send_4xxResponse(405)
		}
	*/
	// std::cout << "is Sent: " << this->resIsSent << std::endl;
	// std::cout << "is Arrived: " << this->reqIsArrived << std::endl;
	(void)host;
	if (this->reqIsArrived) {
		std::string res = "HTTP/1.1 200 OK\n";
		std::string fileName = "public/html/index.html";
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
	std::cout << "request " << GREEN << "done" << RESET << std::endl;
	// std::cout << "request size: " << GREEN << this->request.getSize() << RESET << std::endl;
	// std::cout << "request: " << YELLOW << this->request.getBuffer() << RESET << std::endl;
	this->pollfd->events = POLLIN | POLLOUT | POLLHUP;
}

void	Client::resHasSent()
{
	// std::cout << YELLOW << "resHasSent()" << RESET << std::endl;
	std::cout << "response " << GREEN << "sent" << RESET << std::endl;
	this->pollfd->events = POLLIN | POLLHUP;
}

void	Client::reset()
{
	std::cout << "reset()" << std::endl;
	this->pollfd->events = POLLIN | POLLHUP;
}