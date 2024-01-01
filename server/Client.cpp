#include "Client.hpp"

Client::Client(int fd, struct sockaddr_in address)
	:
	fd(fd),
	address(address),
	pollfd(NULL),
	processing_level(INITIAL),
	isAllowedMethod(false)
{
	// set timout
	this->logtime = 0;
	this->logtime_start = time(0);
	this->response.setSocket(this->fd);
}

Client::~Client() {}

int Client::getFd() const {
	return this->fd;
}

struct sockaddr_in Client::getAddress() const {
	return this->address;
}

void	Client::setServerInfo(std::string port, std::string host, std::string s_name)
{
	this->serverInfo["PORT"] = port;
	this->serverInfo["HOST"] = host;
	this->serverInfo["SERVER_NAME"] = s_name;
}

Request	Client::getRequest() const {
	return this->request;
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

bool	Client::checkLogTime()
{
	this->logtime = time(0) - this->logtime_start;
	if (this->logtime >= CLIENT_TIMEOUT) {
		std::cout << RED << "TIMEOUT PASSED" << RESET << std::endl;
		return true;
	}
	return false;
}

bool		Client::readRequest(struct pollfd *pollfd) {
	setPollfd(pollfd);
	this->logtime = 0;

	char buf[1024] = {0};
	int readed = recv(this->fd, buf, sizeof(buf), 0);
	if (readed == -1 || readed == 0) {
		// this->request.resetBuffer();
		this->reqHasRead();
		// then: close connection
	}


	// std::cout << RED << "before: " << this->request.getStatus() << RESET << std::endl;
	if (this->request.parseRequest(buf, readed, this->fd)) {
		// std::cout << RED << "salat" << RESET << std::endl;
		this->reqHasRead();
		std::cout << "uri: " + this->request.getUri() << std::endl;
		// this->response.setStatus(this->request.getStatus());

		// std::map<std::string, std::string>::iterator it;
		// std::map<std::string, std::string> headers = this->request.getHeaders();
	
		// for (it = headers.begin(); it != headers.end(); it++) {
		// 	std::cout << YELLOW << it->first << " = " << it->second << RESET << std::endl;
		// }
		return true;
	}
	/*
		if (still reading request)
			return false;
		then: request has finished reading, return true
	*/
	// std::cout << YELLOW << "mazal" << RESET << std::endl;
	return false;
}

bool	Client::createResponse(std::vector<Location> &locations) {

	// -> find location that matches with uri
	// std::string str = "/public/html/";
	// this->request.setUri(str);
	Location *location = this->response.findLocation(locations, this->request.getUri());
	// std::cout << this->request.getUri() << std::endl;
	// std::cout << location->getPath() << std::endl;
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
		// this->response.setStatus(200);
		// std::cout << YELLOW << "path: " << location->path << RESET << std::endl;
		// std::cout << YELLOW << "root: " << location->root << RESET << std::endl;
		// std::cout << YELLOW << "redirection: " << location->redirection << RESET << std::endl;
		if (!location || this->response.getStatus() != 200)
			this->response.setResponseType(ERROR);
		else {
			if (!location->getRedirection().empty()) {
				// std::cout << RED << "Is Redirect" << RESET << std::endl;
				this->response.setResponseType(REDIRECT);
			}
			else if (!this->methodIsAllowed(location->allowMethods, this->request.getMethod()))
				this->response.setResponseType(ERROR);
		}
		processing_level = SENDING;
	}
	else if (processing_level == SENDING)
		this->send_response();
	else if (processing_level == PROCESSED) {
		this->resHasSent();
		return true;
	}
	return false;
}

// std::string	getRequestedResource()
// {
// }

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
		bool isResponseEnd = this->response.getMethod(this->request.getUri());
		std::cout << GREEN << "GET METHOD" << RESET << std::endl;
		/*
			if (GET)
				-> perform action, getMethod()
			else if (POST)
				if (upload)
					->upload
				else
					-> GET without cgi
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
	// std::cout << "reset()" << std::endl;
	this->pollfd->events = POLLIN | POLLHUP;
	this->isAllowedMethod = false;
	this->processing_level = INITIAL;
	this->response.reset();
}