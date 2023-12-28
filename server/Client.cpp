#include "Client.hpp"

Client::Client(int fd, struct sockaddr_in address)
	:
	fd(fd),
	address(address),
	pollfd(NULL),
	processing_level(INITIAL),
	isAllowedMethod(false)
{
	this->response.setSocket(this->fd);
}

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

bool	Client::methodIsAllowed(std::vector<std::string> &allowMethods, std::string method)
{
	std::vector<std::string>::iterator it;
	for (it = allowMethods.begin(); it != allowMethods.end(); it++)
	{
		if (method == *it)
			return true;
	}
	this->response.setStatus(405);
	return false;
}

bool		Client::readRequest(struct pollfd *pollfd) {
	setPollfd(pollfd);

	char buf[1024] = {0};
	int readed = recv(this->fd, buf, sizeof(buf), 0);
	if (readed == -1 || readed == 0) {
		// this->request.resetBuffer();
		this->reqHasRead();
		// then: close connection
	}
	this->request.appendToBuffer(buf);
	this->reqHasRead();
	/*
		if (still reading request)
			return false;
		then: request has finished reading, return true
	*/
	return true;
}

void		Client::createResponse(std::vector<Location> &locations) {

	// -> find location that matches with uri
	std::string str = "/";
	this->request.setUri(str);
	Location *location = this->response.findLocation(locations, this->request.getUri());
	/*
		-> find location that matches with uri

		-> get methods allowed

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
	if (processing_level == INITIAL)
	{
		this->response.setLocation(location);
		// -> this line below is to test error pages
		this->response.setStatus(400);
		if (!location || this->response.getStatus() != 200)
			this->response.setResponseType(ERROR);
		else {
			if (!location->getRedirection().empty()) 
				this->response.setResponseType(REDIRECT);
			else if (!this->methodIsAllowed(location->allowMethods, "GET"))
				this->response.setResponseType(ERROR);
		}
		processing_level = SENDING;
	}
	else if (processing_level == SENDING)
		this->send_response();
	else if (processing_level == PROCESSED)
		this->resHasSent();
}

void	Client::send_response()
{
	/*
		if (OK) {
			if (GET)
				-> perform action, getMethod()
			else if (POST)
				-> perform action, postMethod()
			else if (DELETE)
				-> perform action, deleteMethod()
		}
		else if (REDIRECT) {
			this->response.redirect(redirection);
			this->processing_level = PROCESSED;
		}
		else if (ERROR) {
			bool isResponseEnd = this->response.send_response_error();
			this->processing_level = isResponseEnd ? PROCESSED : SENDING;
		}
	*/
	if (this->response.getResponseType() == OK) {
		bool isResponseEnd = false;
		/*
			if (GET)
				-> perform action, getMethod()
			else if (POST)
				-> perform action, postMethod()
			else if (DELETE)
				-> perform action, deleteMethod()
		*/
		this->processing_level = isResponseEnd ? PROCESSED : SENDING;
	}
	else if (this->response.getResponseType() == REDIRECT) {
		this->response.redirect(this->response.getLocation()->getRedirection());
		this->processing_level = PROCESSED;
	}
	else if (this->response.getResponseType() == ERROR) {
		bool isResponseEnd = this->response.send_response_error();
		this->processing_level = isResponseEnd ? PROCESSED : SENDING;
	}
}

void	Client::reqHasRead()
{
	std::cout << "request " << GREEN << "done" << RESET << std::endl;
	// std::cout << "request size: " << GREEN << this->request.getSize() << RESET << std::endl;
	// std::cout << "request: " << YELLOW << this->request.getBuffer() << RESET << std::endl;
	this->pollfd->events = POLLOUT | POLLHUP;
}

void	Client::resHasSent()
{
	// std::cout << YELLOW << "resHasSent()" << RESET << std::endl;
	std::cout << "response " << GREEN << "sent" << RESET << std::endl;
	this->reset();
}

void	Client::reset()
{
	std::cout << "reset()" << std::endl;
	this->pollfd->events = POLLIN | POLLHUP;
	this->isAllowedMethod = false;
	this->processing_level = INITIAL;
	this->response.reset();
}