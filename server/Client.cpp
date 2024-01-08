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
	this->logtime_start = time(0);	this->response.setSocket(this->fd);
}

Client::~Client() {
	// std::cout << BOLDRED << "Client Destructor Called" << RESET << std::endl;
}

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
	this->response.setBodyFileName(this->request.getFilename());
}

void	Client::setFirstCgiEnv(void)
{
	std::string uri = this->request.getUri();
	if (uri[0] == '/')
		uri.erase(0, 1);
	bool	hasQS = hasQueryString(uri);
	this->firstCgiEnv["SERVER_NAME"] = "SERVER_NAME=" + this->serverInfo["SERVER_NAME"];
	this->firstCgiEnv["SERVER_PORT"] = "SERVER_PORT=" + this->serverInfo["PORT"];
	this->firstCgiEnv["SERVER_PROTOCOL"] = "SERVER_PROTOCOL=HTTP/1.1";
	this->firstCgiEnv["HTTP_HOST"] = "HTTP_HOST=" + this->serverInfo["HOST"];
	this->firstCgiEnv["REMOTE_ADDR"] = "REMOTE_ADDR=" + this->serverInfo["HOST"];
	this->firstCgiEnv["HTTP_CONNECTION"] = "HTTP_CONNECTION=" + this->request.getHeader("connection");
	this->firstCgiEnv["HTTP_ACCEPT"] = "HTTP_ACCEPT=" + this->request.getHeader("accept");
	this->firstCgiEnv["HTTP_USER_AGENT"] = "HTTP_USER_AGENT=" + this->request.getHeader("user-agent");
	this->firstCgiEnv["HTTP_COOKIE"] = "HTTP_COOKIE=" + this->request.getHeader("cookie");
	this->firstCgiEnv["REDIRECT_STATUS"] = "REDIRECT_STATUS=0";
	this->firstCgiEnv["REQUEST_URI"] = "REQUEST_URI=" + uri;
	this->firstCgiEnv["DOCUMENT_ROOT"] = "DOCUMENT_ROOT=" + this->location->getRoot();

	long long pos = uri.find(this->location.getCgiExec()[1] + this->location.getCgiExec()[1].length());
	if (pos != -1)
	{
		this->firstCgiEnv["SCRIPT_NAME"] = "SCRIPT_NAME=" + uri.substr(0, pos);
		if (hasQS)
		{
			this->firstCgiEnv["PATH_INFO"] = "PATH_INFO=" + uri.substr(pos, uri.find('?') - pos); // pos + 1 I think
			this->firstCgiEnv["PATH_TRANSLATED"] = "PATH_TRANSLATED=" + this->location->getRoot() + uri.substr(pos, uri.find('?') - pos);
		}
		else
		{
			this->firstCgiEnv["PATH_INFO"] = "PATH_INFO=" + uri.substr(pos); // pos + 1 I think
			this->firstCgiEnv["PATH_TRANSLATED"] = "PATH_TRANSLATED=" + this->location->getRoot() + uri.substr(pos);
		}
	}
	if (this->request.getMethod() == "GET")
	{
		this->firstCgiEnv["REQUEST_METHOD=GET"];

		if (hasQS)
			this->firstCgiEnv["QUERY_STRING"] = "QUERY_STRING=" + uri.substr(uri.find('?') + 1);
	}
	else if (this->request.getMethod() == "POST")
	{
		this->firstCgiEnv["REQUEST_METHOD"] = "REQUEST_METHOD=POST";
		this->firstCgiEnv["CONTENT_TYPE"] = "CONTENT_TYPE=" + this->request.getHeader("content-type");
		this->firstCgiEnv["CONTENT_LENGTH"] = "CONTENT_LENGTH=" + this->request.getHeader("content-length");
	}
}

Request	Client::getRequest() const {
	return this->request;
}

void	Client::setPollfd(struct pollfd	*pollfd) {
	this->pollfd = pollfd;
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


	// std::cout << RED << "before: " << this->request.getStatus() << RESET << std::endl;git p
	if (this->request.parseRequest(buf, readed, this->fd)) {
		// std::cout << RED << "salat" << RESET << std::endl;
		std::cout << "uri: " + this->request.getUri() << std::endl;
		this->reqHasRead();
		return true;
	}
	return false;
}

bool	Client::createResponse(std::vector<Location> &locations) {
	// log_level();
	// -> find location that matches with uri
	// std::string str = "/public/html/";
	// this->request.setUri(str);
	std::string uri = this->request.getUri();
	Location *location = this->response.findLocation(locations, uri);
	// std::cout << uri << std::endl;
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
		// this->response.setStatus(400);
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
			else if (!this->methodIsAllowed(location->getAllowMethods(), this->request.getMethod()))
				this->response.setResponseType(ERROR);
		}
		processing_level = SENDING;
	}
	if (processing_level == SENDING)
		this->send_response();
	if (processing_level == PROCESSED) {
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
		try
		{
			// isResponseEnd = this->response.getMethod(this->request.getUri(), this->request.getHeaders());
			// this->response.log_res_level();
			this->setFirstCgiEnv();
			bool isResponseEnd = false;
			if (this->request.getMethod() == "GET") {

				isResponseEnd = this->response.newGet(this->request.getUri(), this->firstCgiEnv, GET);
			}
			else if (this->request.getMethod() == "POST") {
				isResponseEnd = this->response.uploadPostMethod(this->request);
			}
			// todo: DELETE Method
			// ...
			this->processing_level = isResponseEnd ? PROCESSED : SENDING;
			// this->response.log_res_level();
			// this->response.log_res_type();
		}
		catch (int error_code)
		{
			this->response.setStatus(error_code);
			this->response.setResponseType(ERROR);
		}
	}
	if (this->response.getResponseType() == REDIRECT) {
		this->response.redirect(this->response.getLocation()->getRedirection());
		this->processing_level = PROCESSED;
	}
	if (this->response.getResponseType() == ERROR) {
		bool isResponseEnd = this->response.send_response_error();
		this->processing_level = isResponseEnd ? PROCESSED : SENDING;
	}
}

void	Client::reqHasRead()
{
	std::cout << "request " << GREEN << "done" << RESET << std::endl;
	this->pollfd->events = POLLOUT | POLLHUP;
	// this->request.reset();
}

void	Client::resHasSent()
{
	// std::cout << YELLOW << "resHasSent()" << RESET << std::endl;
	std::cout << "response " << GREEN << "sent" << RESET << std::endl;
	this->pollfd->events = POLLIN | POLLHUP;
	this->reset();
	this->response.reset();
	this->request.reset();
}

void	Client::reset()
{
	// std::cout << "reset()" << std::endl;
	this->pollfd->events = POLLIN | POLLHUP;
	this->isAllowedMethod = false;
	this->processing_level = INITIAL;
	this->logtime = 0;
	this->logtime_start = time(0);
}

// title : log methods

void Client::log() {
	std::cout << "client with fd: " << this->fd << std::endl;
}

void	Client::log_level() {
	std::cout << "level: " << CYAN;
	switch (this->processing_level)
	{
		case INITIAL:
			std::cout << "INITIAL" << std::endl;
			break;
		case SENDING:
			std::cout << "SENDING" << std::endl;
			break;
		case PROCESSED:
			std::cout << "PROCESSED" << std::endl;
			break;
	}
	std::cout << RESET;
}