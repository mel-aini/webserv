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

bool	hasQueryString(std::string uri)
{
	size_t	i;
	for (i = 0; i < uri.length(); i++)
		if (uri[i] == '?')
			break ;
	if (i == uri.length())
		return (false);
	return (true);
}

char**	Client::getCgiEnv(int method_type)
{
	char	**env = new char*[12];
	// std::memset(env, 0, 12);
	std::string uri = this->request.getUri();
	std::string	variable;

	variable = "SERVER_NAME=" + this->serverInfo["SERVER_NAME"];
	std::cout << variable << std::endl;
	env[0] = strdup(variable.c_str());

	variable = "SERVER_PORT=" + this->serverInfo["PORT"];
	std::cout << variable << std::endl;
	env[1] = strdup(variable.c_str());

	variable = "SERVER_PROTOCOL=HTTP/1.1";
	std::cout << variable << std::endl;
	env[2] = strdup(variable.c_str());

	if (uri[0] == '/')
		uri.erase(0, 1);
	variable = "SCRIPT_NAME=/" + uri.substr(0, uri.find(".php") + 4);
	std::cout << variable << std::endl;
	env[3] = strdup(variable.c_str());
	
	variable = "PATH_INFO=" + uri.substr(uri.find(".php") + 4);
	std::cout << variable << std::endl;
	env[4] = strdup(variable.c_str());

	variable = "HTTP_ACCEPT=" + this->request.getHeaderLine("accept");
	std::cout << variable << std::endl;
	env[5] = strdup(variable.c_str());

	variable = "HTTP_USER_AGENT=" + this->request.getHeaderLine("aser-agent");
	std::cout << variable << std::endl;
	env[6] = strdup(variable.c_str());

	variable = "REQUEST_URI=" + uri;
	std::cout << variable << std::endl;
	env[7] = strdup(variable.c_str());

	variable = "STATUS=200";
	std::cout << variable << std::endl;
	env[8] = strdup(variable.c_str());

	if (method_type == GET)
	{
		variable = "REQUEST_METHOD=GET";
		std::cout << variable << std::endl;
		env[9] = strdup(variable.c_str());

		if (hasQueryString(uri))
		{
			std::string queryString = uri.substr(uri.find('?') + 1);
			variable = "QUERY_STRING=" + queryString;
			std::cout << variable << std::endl;
			env[10] = strdup(variable.c_str());

			env[11] = NULL;
		}
		else
		{
			env[10] = NULL;
			env[11] = NULL;
		}
	}
	else if (method_type == POST)
	{
		variable = "REQUEST_METHOD=POST";
		std::cout << variable << std::endl;
		env[9] = strdup(variable.c_str());

		variable = "CONTENT_TYPE=" + this->request.getHeaderLine("content-type");
		std::cout << variable << std::endl;
		env[10] = strdup(variable.c_str());

		variable = "CONTENT_LENGTH=" + this->request.getHeaderLine("content-length");
		std::cout << variable << std::endl;
		env[11] = strdup(variable.c_str());
	}
	return (env);
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
	std::string uri = this->request.getUri();
	if (hasQueryString(uri))
		uri = uri.substr(0, uri.find('?'));
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
		// std::string uri = this->request.getUri();
		// if (hasQueryString(uri))
		// 	uri = uri.substr(0, uri.find('?'));
		// bool isResponseEnd = false;
		// long long pos = uri.find(".php");
		// if (this->response.getLocation()->getCgiExec().size() != 0 && pos != -1) {
		// 	std::cout << "hana 1\n";
		// 	isResponseEnd = this->response.getMethod(uri, this->getCgiEnv(GET));
		// }
		// else {
		// 	std::cout << "hana 2\n";
		// 	isResponseEnd = this->response.getMethod(uri, NULL);
		// }
		std::cout << GREEN << "GET METHOD" << RESET << std::endl;
		bool isResponseEnd = this->response.uploadPostMethod(this->request);
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
	//this->request.reset();
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