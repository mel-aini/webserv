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
	/*
		read 1024, still 1000
		read 1000, 
	*/
	// std::cout << RED << buf << RESET << std::endl;
	this->request.appendToBuffer(buf);
	// std::cout << buf;
	// this->request.resetBuffer();
	this->reqHasRead();
	/*
		if (still reading request)
			return false;
		then: request has finished reading, return true
	*/
	return true;
}

void		Client::createResponse(std::vector<Location> &locations) {
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
		this->response.setLocation(&locations[0]);
		Location *location = this->response.getLocation();
		// -> this line below is to test error pages
		this->response.setStatus(403);
		if (!location || this->response.getStatus() != 200) {
			/*
				then: no matching location
				!location case:
					-> this->response.setStatus(400);
			*/
			this->response.send_response_error();
			this->processing_level = PROCESSED;
			return;
		}
		const std::string &uri = location->getRedirection();
		if (this->response.getStatus() != 200) {
			this->response.send_response_error();
		}
		else if (!uri.empty()) {
			// then: location has a redirect
			this->response.redirect(uri);
			this->processing_level = PROCESSED;
			return;
		}
		processing_level = SENDING;
		if (this->methodIsAllowed(location->allowMethods, "GOT"))
			this->isAllowedMethod = true;
	}
	else if (processing_level == SENDING)
		this->executeMethods();
	else if (processing_level == PROCESSED)
		this->resHasSent();
}

void	Client::executeMethods()
{
	if (this->isAllowedMethod) {
		/*
			if (GET)
				-> perform action, getMethod()
			else if (POST)
				-> perform action, postMethod()
			else if (DELETE)
				-> perform action, deleteMethod()
		*/
	}
	else {
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