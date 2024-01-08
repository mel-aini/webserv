#include "Client.hpp"

Client::Client(int fd, struct sockaddr_in address)
	:
	fd(fd),
	address(address),
	pollfd(NULL),
	processing_level(INITIAL),
	isAllowedMethod(false),
	location(NULL)
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
	long long	pos;
	std::string uri = this->request.getUri();
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

	if (this->location->getCgiExec().size() == 0)
		pos = -1;
	else
		pos = uri.find(this->location->getCgiExec()[1]) + this->location->getCgiExec()[1].length();

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
	// if (uri[0] == '/')
	// 	uri.erase(0, 1);
	if (this->request.getMethod() == "GET")
	{
		this->firstCgiEnv["REQUEST_METHOD"] = "REQUEST_METHOD=GET";

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

bool	compareByLength(Location& a, Location& b)
{
    return (a.getPath().length() > b.getPath().length());
}

bool	Client::findLocation(std::vector<Location> &locations, std::string uri)
{
	std::vector<Location>::iterator	it;
	std::string tmp = hasQueryString(uri) ? uri.substr(0, uri.find('?')) : uri;

	if (tmp.empty() || tmp[0] != '/') {
		this->response.setStatus(400);
		this->location = NULL;
		return false;
	}

	if (locations.size() > 1)
		std::sort(locations.begin(), locations.end(), compareByLength);
	if (tmp[tmp.length() - 1] == '/')
	{
		if (tmp[tmp.length() - 2] == '/') {
			this->response.setStatus(400);
			this->location = NULL;
			return false;
		}
		tmp.erase(tmp.length() - 1);
	}

	long long pos;

	while (true)
	{
		it = locations.begin();
		for (; it != locations.end(); it++)
			if (tmp == it->getPath())
				break ;
		if (it != locations.end())
			break ;
		pos = tmp.rfind('/');
		if (pos == -1 || pos == 0)
			break ;
		tmp = tmp.substr(0, pos);
	}
	if (it == locations.end())
		it = locations.end() - 1;
	this->location = &(*it);
	return true;
}

bool        Client::isBeyondMaxBodySize() {
	if (location && this->request.getBodysize() > (location->clientMaxBodySize / 1000000)) {
		this->response.setStatus(413);
		this->response.setResponseType(ERROR);
		return true;
	}
	return false;
}

bool		Client::readRequest(std::vector<Location> &locations) {
	this->logtime = 0;

	char buf[1024] = {0};

	int readed = recv(this->fd, buf, sizeof(buf), 0);
	if (readed == -1 || readed == 0) {
		this->reqHasRead();
		return true;
		// then: close connection
		// throw RequestFailed();
	}
	if (!this->location && this->request.getState() > METHOD) {
		if(!findLocation(locations, this->request.getUri())) {
			this->reqHasRead();
			return true;
		}
	}

	bool isReadEnd = this->request.parseRequest(buf, readed, this->fd);

	if (isBeyondMaxBodySize()) {
		this->reqHasRead();
		return true;
	}

	if (isReadEnd) {

		this->reqHasRead();
		if (!this->location)
			findLocation(locations, this->request.getUri());
		
		return true;
	}
	return false;
}

bool	Client::createResponse() {
	// log_level();
	// -> find location that matches with uri


	if (processing_level == INITIAL)
	{
		this->response.setLocation(location);
		if (!location || this->response.getStatus() != 200)
			this->response.setResponseType(ERROR);
		else {
			if (!location->getRedirection().empty()) {
				this->response.setResponseType(REDIRECT);
			}
			else if (!this->methodIsAllowed(location->getAllowMethods(), this->request.getMethod()))
				this->response.setResponseType(ERROR);
		}
		processing_level = SENDING;
	}
	if (processing_level == SENDING)
		this->send_response();

	return processing_level == PROCESSED;
}

// std::string	getRequestedResource()
// {
// }

void	Client::send_response()
{
	if (this->response.getResponseType() == OK) {
		try
		{
			// todo: DELETE Method
			// todo complete: POST Method

			this->setFirstCgiEnv();
			bool isResponseEnd = false;
			if (this->request.getMethod() == "GET")
				isResponseEnd = this->response.get_method(this->request.getUri(), this->firstCgiEnv, GET);
			else if (this->request.getMethod() == "POST")
				isResponseEnd = this->response.post_method(this->request, this->request.getHeaders(), POST);
			else if (this->request.getMethod() == "DELETE")
				isResponseEnd = this->response.delete_method(this->request.getUri());
			this->processing_level = isResponseEnd ? PROCESSED : SENDING;
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
}

void	Client::resHasSent()
{
	std::cout << "response " << GREEN << "sent" << RESET << std::endl;

	this->pollfd->events = POLLIN | POLLHUP;
	this->reset();
	this->response.reset();
	this->request.reset();
}

void	Client::reset()
{
	this->pollfd->events = POLLIN | POLLHUP;
	this->isAllowedMethod = false;
	this->processing_level = INITIAL;
	this->location = NULL;
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