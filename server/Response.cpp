#include "Response.hpp"

Response::Response() 
	:
	status(200), 
	location(NULL),
	sending_level(GET_REQUESTED_RES),
	method_level(FINDRESOURCE),
	request_case(NO_CASE),
	response_type(OK),
	match_index(NO),
	bodyOffset(0),
	fileOffset(0),
	index(0),
	sendingFile(false),
	fileToUpload("")
{
	status_codes[200] = "OK";
	status_codes[201] = "Created";
	status_codes[204] = "No Content";
	status_codes[301] = "Moved Permanently";
	status_codes[400] = "Bad Request";
	status_codes[403] = "Forbidden";
	status_codes[404] = "Not Found";
	status_codes[405] = "Method Not Allowed";
	status_codes[411] = "Length Required";
	status_codes[413] = "Payload Too Large";
	status_codes[414] = "URI Too Long";
	status_codes[500] = "Internal Server Error";
	status_codes[501] = "Not Implemented";
	status_codes[502] = "Bad Gateway";
	status_codes[505] = "HTTP Version Not Supported";

	content_type[".html"] = "text/html";
	content_type[".htm"] = "text/html";
	content_type[".css"] = "text/css";
	content_type[".jpeg"] = "image/jpeg";
	content_type[".jpg"] = "image/jpeg";
	content_type[".mp3"] = "audio/mpeg";
	content_type[".mp4"] = "video/mp4";
	content_type[".png"] = "image/png";
	content_type[".js"] = "text/javascript";
	content_type[".php"] = "application/x-httpd-php";
}

Response::~Response() {
	// std::cout << BOLDRED << "Response Destructor Called" << RESET << std::endl;
}

void	Response::setServerInfo(std::map<std::string, std::string> serverInfo)
{
	this->serverInfo = serverInfo;
}

int	Response::getStatus() const {
	return this->status;
}

void	Response::setStatus(unsigned int status) {
	this->status = status;
}

void	Response::setLocation(Location *location) {
	this->location = location;
}

Location    *Response::getLocation() {
	return this->location;
}

unsigned int	Response::getResponseType() const {
	return this->response_type;
}

unsigned int        Response::getSendingLevel() const {
	return this->sending_level;
}

void                Response::setSendingLevel(unsigned int level) {
	this->sending_level = level;
}

void	Response::setResponseType(unsigned int response_type)
{
	this->response_type = response_type;
}

std::string Response::getStatusMessage() {
	return this->status_codes[this->status];
}

void	Response::setSocket(int fd) {
	this->socket = fd;
}

bool	Response::isInErrorPages()
{
	std::vector<std::pair<std::string, std::vector<int> > >::iterator	it;

	for (it = location->errorPages.begin(); it != location->errorPages.end(); it++) {
		std::vector<int>::iterator it2;
		for (it2 = it->second.begin(); it2 != it->second.end(); it2++) {
			if (this->status == (unsigned int)*it2) {
				this->errPage = it->first;
				return true;
			}
		}
	}
	return false;
}

// ... working on
bool	Response::sendFile(std::string fileName)
{
	char buf[4000] = {0};

	if (bodyOffset == 0)
		std::cout << BOLDWHITE << "fileName: " + fileName << RESET << std::endl;

	std::ifstream file(fileName.c_str(), std::ios::binary | std::ios::in);
	if (!file.is_open()) {
		std::cerr << BOLDRED << "Error: Unable to open infile" << RESET << std::endl;
		throw ResponseFailed();
	}
	file.seekg(this->bodyOffset, std::ios::beg);

	file.read(buf, sizeof(buf));
	int bytesRead = file.gcount();
	// std::cout << "bodyOffset: " << RED << bodyOffset << RESET << std::endl;
	int s = send(this->socket, buf, bytesRead, 0);
	if (s == -1)
		throw ResponseFailed();
	bodyOffset += s;
	// std::cout << "send: " << BOLDCYAN << static_cast<double>(bodyOffset) / 1000000 << "Mb" << RESET << std::endl;
	if (file.eof()) {
		this->sending_level = SENDING_END;
		// std::cout << "bytes sent: " << BOLDGREEN << static_cast<double>(bodyOffset) / 1000000 << "Mb" << RESET << std::endl;
		file.close();
		std::cout << BOLDGREEN << "Reached end of file" << RESET << std::endl;
		return true;
	}
	file.close();
	return false;
}

bool	Response::send_response_error()
{
	// std::cout << MAGENTA << "Here!" << RESET << std::endl;
	if (this->sending_level == GET_REQUESTED_RES)
		this->sending_level = SENDING_HEADERS;
	if (this->sending_level == SENDING_HEADERS)
	{
		std::stringstream sizestream;
		if (this->location && this->isInErrorPages())
		{
			std::string fileName = this->location->root + "/" + errPage;
			std::ifstream file(fileName.c_str(), std::ios::binary | std::ios::in);
			if (file.is_open()) {
				struct stat fileInfo;
				if (stat(fileName.c_str(), &fileInfo) == 0) {
					sizestream << fileInfo.st_size;
					this->sendingFile = true;
				}
			}
		}
		if (!this->sendingFile) {
			std::string message = this->getStatusMessage();
			HtmlTemplate htmlErrorPage(this->status, message);
			sizestream << htmlErrorPage.getHtml().size();
		}
		this->headers["Content-Type: "] = "text/html";
		this->headers["Content-Length: "] = sizestream.str();

		send_status_line_and_headers();
		this->sending_level = SENDING_BODY;
	}
	if (this->sending_level == SENDING_BODY)
	{
		if (this->sendingFile) {
			if (this->sendFile(this->location->root + "/" + this->errPage))
				return true;
			return false;
		}
		else {
			std::string message = this->getStatusMessage();
			HtmlTemplate htmlErrorPage(this->status, message);

			const std::string& response = htmlErrorPage.getHtml();

			const char *buf = response.c_str();
			if (send(this->socket, buf, response.size(), 0) == -1)
				throw ResponseFailed();

			this->sending_level = SENDING_END;
		}
	}
	else if (this->sending_level == SENDING_END)
		return true;
	return false;
}

bool	Response::send_response_index_files(std::string uri)
{
	if (uri[0] == '/')
		uri.erase(0, 1);

	std::string	target = this->location->getRoot() + uri;

	std::cout << BOLDRED << "---- target: " << target << std::endl;
	DIR *dir = opendir(target.c_str());
	if (!dir)
		throw 500;

	struct dirent *dirContent;
	std::vector<std::string> content;
	while ((dirContent = readdir(dir)) != NULL)
		content.push_back(dirContent->d_name);

	std::stringstream	sizestream;
	HtmlTemplate htmlErrorPage(target, content);
	sizestream << htmlErrorPage.getHtml().size();
	this->headers["Content-Type: "] = "text/html";
	this->headers["Content-Length: "] = sizestream.str();

	send_status_line_and_headers();

	const std::string& response = htmlErrorPage.getHtml();

	const char *buf = response.c_str();
	if (send(this->socket, buf, response.size(), 0) == -1)
		throw ResponseFailed();

	this->sending_level = SENDING_END;
	return true;
}

void	Response::send_status_line_and_headers()
{
	// this->log_members();
	// title: prepare status line
	std::stringstream str;
	str << this->status;
	std::string status = str.str();
	this->message = this->status_codes[this->status];
	std::string status_line = "HTTP/1.1 " + status + " " + this->message + "\r\n";

	std::string headers;
	std::map<std::string, std::string>::iterator it = this->headers.begin();
	while (it != this->headers.end()) {
		headers += it->first;
		headers += it->second;
		it++;
		if (it != this->headers.end())
			headers += "\r\n";
	}

	std::string response = status_line + headers + "\r\n\r\n";
	// std::cout << YELLOW << response << RESET << std::endl; 
	// std::cout << YELLOW << response << RESET << std::endl;
	const char *buf = response.c_str();
	if (send(this->socket, buf, response.size(), 0) == -1)
		throw ResponseFailed();
}

void    Response::redirect(const std::string& location)
{
	// then: send a 301 Moved Permanently with uri in <Location> header
	this->status = 301;
	this->headers["Location: "] = location;
	send_status_line_and_headers();
	// std::cout << "REDIRECT" << std::endl;
}

bool	compareByLength(Location& a, Location& b)
{
    return (a.getPath().length() > b.getPath().length());
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

Location *Response::findLocation(std::vector<Location> &locations, std::string uri)
{
	std::vector<Location>::iterator	it;
	std::string tmp = hasQueryString(uri) ? uri.substr(0, uri.find('?')) : uri;

	if (tmp.empty() || tmp[0] != '/') {
		this->setStatus(400);
		return NULL;
	}

	if (locations.size() > 1)
		std::sort(locations.begin(), locations.end(), compareByLength);
	if (tmp[tmp.length() - 1] == '/')
	{
		if (tmp[tmp.length() - 2] == '/') {
			this->setStatus(400);
			return NULL;
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
	return &(*it);
}


bool	Response::getRequestedResource(std::string uri)
{
	memset(&this->fileInf, 0, sizeof(this->fileInf));
	uri = uri.substr(0, uri.find(".php") + 4);
	if (uri[0] == '/')
		uri.erase(0, 1);
	if (uri[uri.length() - 1] == '/')
		uri.erase(uri.length() - 1, 1);
	if (stat(uri.c_str(), &this->fileInf) == 0)
	if (access(uri.c_str(), F_OK | R_OK) == 0)
	{
		if (stat(uri.c_str(), &this->fileInf) == 0)
		{
			this->request_case = OTHER_CASE;
			return (true);
		}
	}
	std::string	fileCase = this->location->getRoot() + uri;
	std::cout << BOLDMAGENTA << "REQ_RES: " << RESET << fileCase << RESET << std::endl;
	if (stat(fileCase.c_str(), &this->fileInf) == 0)
	{
		std::cout << GREEN << "FOUND" << RESET << std::endl;
		if (S_ISREG(this->fileInf.st_mode))
		{
			this->request_case = FILE_CASE;
			return (true);
			if (S_ISREG(this->fileInf.st_mode))
			{
				this->request_case = OTHER_CASE;
				return (true);
			}
		}
	}
	std::string	path = this->location->getRoot() + "/" + uri;
	if (access(path.c_str(), F_OK | R_OK) == 0)
	{
		if (stat(path.c_str(), &this->fileInf) == 0)
		{
			if (S_ISDIR(this->fileInf.st_mode))
			{
				this->request_case = DIR_CASE;
				return (true);
			}
			else if (S_ISREG(this->fileInf.st_mode))
			{
				this->request_case = FILE_CASE;
				return (true);
			}
		}
	}
	return (false);
}


void	Response::setError(int status_code) {
	this->status = status_code;
	this->response_type = ERROR;
}

// ... working on
bool	Response::isFileExist(std::string& target) {
	if (access(target.c_str(), F_OK) == 0) { // then: exist 
		if (access(target.c_str(), R_OK) == 0) { // then: has permission
			return true;
		}
		else {
			throw 403;
		}
	}
	return false;
}

// ... working on
bool	Response::isTarget(std::string& target,  struct stat *fileInfo) {
	if (stat(target.c_str(), fileInfo) == 0)
	{
		if (S_ISREG(fileInfo->st_mode)) {
			this->fileToSend = target;
			return true;
		}
		return false;
	}
	throw 404;
}

// ... working on
bool	Response::getRequestedFile(std::string uri)
{
	if (uri[0] == '/')
		uri.erase(0, 1);

	std::string	target = this->location->getRoot() + uri;

	std::cout << BOLDRED << "target: " << target << RESET << std::endl;
	if (!this->isFileExist(target)) {
		// std::cout << BOLDRED << "Here!!! 1" << RESET << std::endl;
		throw 404;
	}

	struct stat fileInfo;

	if (this->isTarget(target, &fileInfo)) {
		// std::cout << BOLDRED << "isTarget" << RESET << std::endl;
		return true;
	}
	else if (S_ISDIR(fileInfo.st_mode)) {
		if (uri.back() != '/' && uri.length() != 0) {
			redirect("/" + uri + "/");
			this->sending_level = SENDING_END;
			return true;
		}
		std::vector<std::string>::iterator it;
		std::vector<std::string> index = this->location->getIndex();

		std::cout << "before: " << target << std::endl;
	
		for (it = index.begin(); it != index.end(); it++) {
			std::string target2 = target + *it;

			std::cout << "new target: " << target2 << std::endl;
		
			if (this->isFileExist(target2)) {
				struct stat fileInfo2;
				if (!this->isTarget(target2, &fileInfo2)) {
					// std::cout << BOLDRED << "Here!!! 2" << RESET << std::endl;
					throw 403;
				}
				std::cout << "file to send: " << "[" + this->fileToSend + "]" << std::endl;
				return true;
			}
		}
		// std::cout << BOLDRED << "Here!!! 3" << RESET << std::endl;
		// std::cout << BOLDRED << "No Index" << RESET << std::endl;
		return false;
	}
	std::cout << "file to send: " << "[" + this->fileToSend + "]" << std::endl;
	return false;
}

///////////////////////

char**	Response::getCgiEnv(int method_type, std::string uri, std::map <std::string, std::string> _headers)
{
	size_t size = (hasQueryString(uri) && method_type == GET) ? 13 : 12;
	size = (method_type == POST) ? size : 14;
	char	**env = new char*[size];

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

	if (hasQueryString(uri))
	{
		variable = "PATH_INFO=" + uri.substr(uri.find(".php") + 4, uri.find('?') - (uri.find(".php") + 4));
		std::cout << variable << std::endl;
		env[3] = strdup(variable.c_str());
	}
	else
	{
		variable = "PATH_INFO=" + uri.substr(uri.find(".php") + 4);
		std::cout << variable << std::endl;
		env[3] = strdup(variable.c_str());
	}

	variable = "HTTP_ACCEPT=" + _headers["accept"];
	std::cout << variable << std::endl;
	env[4] = strdup(variable.c_str());

	variable = "HTTP_USER_AGENT=" + _headers["user-agent"];
	std::cout << variable << std::endl;
	env[5] = strdup(variable.c_str());

	variable = "HTTP_COOKIE=" + _headers["cookie"];
	std::cout << variable << std::endl;
	env[6] = strdup(variable.c_str());

	variable = "REDIRECT_STATUS=0";
	std::cout << variable << std::endl;
	env[7] = strdup(variable.c_str());

	variable = "REQUEST_URI=" + uri;
	std::cout << variable << std::endl;
	env[8] = strdup(variable.c_str());

	if (uri[0] == '/')
		uri.erase(0, 1);
	variable = "SCRIPT_FILENAME=" + this->location->getRoot() + uri.substr(0, uri.find(".php") + 4);
	std::cout << variable << std::endl;
	env[9] = strdup(variable.c_str());

	if (method_type == GET)
	{
		variable = "REQUEST_METHOD=GET";
		std::cout << variable << std::endl;
		env[10] = strdup(variable.c_str());

		if (hasQueryString(uri))
		{
			std::string queryString = uri.substr(uri.find('?') + 1);
			variable = "QUERY_STRING=" + queryString;
			std::cout << variable << std::endl;
			env[11] = strdup(variable.c_str());

			env[12] = NULL;
		}
		else
		{
			env[11] = NULL;
		}
	}
	else if (method_type == POST)
	{
		variable = "REQUEST_METHOD=POST";
		std::cout << variable << std::endl;
		env[10] = strdup(variable.c_str());

		variable = "CONTENT_TYPE=" + headers["content-type"];
		std::cout << variable << std::endl;
		env[11] = strdup(variable.c_str());

		variable = "CONTENT_LENGTH=" + headers["content-length"];
		std::cout << variable << std::endl;
		env[12] = strdup(variable.c_str());
		env[13] = NULL;
	}
	return (env);
}

void	freeEnv(char **env)
{
	size_t i = 0;

	while (env[i++])
		free(env[i]);
	free(env);
}

void	Response::executeCgi(std::string uri, std::map <std::string, std::string> _headers, int method_type)
{
	char **env = this->getCgiEnv(method_type, uri, _headers);
	pid_t	pid = fork();
	if (pid == -1)
		throw 502;
	int fdes = open("/tmp/result", O_CREAT | O_RDWR | O_TRUNC, 0666);
	// std::cout << fdes << std::endl;
	if (fdes == -1)
		throw 502;
	if (pid == 0)
	{
		dup2(fdes, 1);
		close(fdes);
		execve(this->location->getCgiExec()[0].c_str(), NULL, env);
		throw 502;
	}
	wait(0);
	freeEnv(env);
	close(fdes);
	std::ifstream result("/tmp/result");
	if (!result.is_open())
		throw 502;
	std::string header;
	std::string body;
	std::string tmp;
	std::string status;
	bool	hasContentLength = 0;
	size_t	contentLength = 0;
	bool	hasContentType = 0;
	while (getline(result, tmp, '\n'))
	{
		if (tmp == "\r")
			break ;
		if (tmp.substr(0, 14) == "Content-type: ")
			hasContentType = 1;
		if (tmp.substr(0, 16) == "Content-length: ")
			hasContentLength = 1;
		if (tmp.substr(0, 8) == "Status: ")
			status = tmp.substr(8);
		else
			header += tmp + "\n";
	}
	if (status.empty())
		status = "HTTP/1.1 200 OK \r\n";
	else
		status = "HTTP/1.1 " + status + "\r\n";
	if (!hasContentType)
		header += "Content-type: text/html\r\n";
	header = status + header;
	while (getline(result, tmp, '\n'))
		body += tmp + "\n";
	result.close();
	contentLength = body.size();
	if (!hasContentLength)
	{
		std::stringstream ss;
		ss << contentLength;
		std::string size;
		ss >> size;
		header += "Content-length: " + size + "\r\n";
	}
	header += "\r\n";
	std::string	response = header + body;
	// std::ofstream file("file");
	// if (file.is_open())
	// {
	// 	file << response;
	// 	file.close();
	// }
	// int fds = open("file", O_RDONLY);
	// char *buf = new char [response.length() + 1];
	// read(fds, buf, response.length());
	// buf[response.length()] = '\0';
	const char *buf = response.c_str();
	if (send(this->socket, buf, response.length(), 0) == -1)
		throw ResponseFailed();
}

bool	Response::post_method(Request &request, std::map <std::string, std::string> _headers, int method_type) {
	if (!location->acceptUpload)
		throw 403;
	(void)method_type;
	(void)_headers;
	return this->uploadPostMethod(request);
}

bool	Response::delete_method(std::string uri) {
	(void)uri;
	return false;
}

// ... working on
bool	Response::get_method(std::string uri, std::map <std::string, std::string> _headers, int method_type) {
	if (this->sending_level == GET_REQUESTED_RES) {
		bool isNoIndex = getRequestedFile(uri);
		if (sending_level == SENDING_END)
			return true;

		if (isNoIndex)
			this->sending_level = SENDING_HEADERS;
		if (this->location->getCgiExec().size() != 0 && this->fileToSend.substr(this->fileToSend.rfind('.')) == this->location->getCgiExec()[1])
		{
			this->executeCgi(uri, _headers, method_type);
			this->sending_level = SENDING_END;
		}
		if (!isNoIndex) {
			// then: autoIndex
			if (!this->location->getAutoIndex())
				throw 403;
			this->send_response_index_files(uri);
			this->sending_level = SENDING_END;
		}
	}
	else if (this->sending_level == SENDING_HEADERS) {
		std::ifstream file(this->fileToSend.c_str(), std::ios::binary | std::ios::in);
		if (!file.is_open())
			throw 500;

		std::stringstream sizestream;
		struct stat fileInfo;
		if (stat(this->fileToSend.c_str(), &fileInfo) == 0) {
			sizestream << fileInfo.st_size;
			// std::cout << "Content-Length: " << RED << sizestream.str() << RESET << std::endl;
		}
		this->headers["Content-Type: "] = getContentType(this->fileToSend);
		this->headers["Content-Length: "] = sizestream.str();
		this->headers["Accept-Ranges: "] = "none";
		// this->headers["Connection: "] = "keep-alive";
		// this->headers["Keep-Alive: "] = "timeout=100000, max=10000";
		// this->headers["Keep-Alive: "] = "timeout=100, max=100";
		send_status_line_and_headers();
		this->sending_level = SENDING_BODY;
	}
	else if (this->sending_level == SENDING_BODY) {
		/*
			todo: cgi design
			if (location has cgi)
				run cgi
			else
				return file
		*/
		return this->sendFile(this->fileToSend);
	}
	else if (this->sending_level == SENDING_END)
		return true;
	return false;
}

std::string	Response::getContentType(std::string path)
{
	std::string	extension = path.substr(path.rfind('.'));
	if (this->content_type[extension].empty())
		return ("");
	return (this->content_type[extension]);
}

void	Response::reset() {
	this->status = 200;
	this->message = this->status_codes[status];
	this->sending_level = GET_REQUESTED_RES;
	this->method_level = FINDRESOURCE;
	this->request_case = NO_CASE;
	this->response_type = OK;
	this->match_index = NO;
	this->headers.clear();
	this->body = "";
	this->location = NULL;
	this->bodyOffset = 0;
	this->sendingFile = false;
	this->fileToSend = "";
}

// title: exceptions

const char	*Response::ResponseFailed::what() const throw() {
	return "Error occured while sending response";
}

// title: log methods

void    Response::log_members() {
	std::cout << "bodyOffset: " << YELLOW << bodyOffset << RESET << std::endl;
}

void    Response::log_res_type()
{
	std::cout << "response type: " << YELLOW;
	switch (this->response_type)
	{
		case OK:
			std::cout << "OK" << std::endl;
			break;
		case REDIRECT:
			std::cout << "REDIRECT" << std::endl;
			break;
		case ERROR:
			std::cout << "ERROR" << std::endl;
			break;
	}
	std::cout << RESET;
}

void    Response::log_res_level()
{
	std::cout << "response level: " << YELLOW;
	switch (this->sending_level)
	{
		case SENDING_HEADERS:
			std::cout << "SENDING_HEADERS" << std::endl;
			break;
		case SENDING_BODY:
			std::cout << "SENDING_BODY" << std::endl;
			break;
		case SENDING_END:
			std::cout << "SENDING_END" << std::endl;
			break;
	}
	std::cout << RESET;
}

void	Response::log()
{
	if (this->sending_level == SENDING_HEADERS) {
		std::cout << CYAN << "sending_level = SENDING_HEADERS" << RESET << std::endl;
	}
	else if (this->sending_level == SENDING_BODY) {
		std::cout << CYAN << "sending_level = SENDING_BODY" << RESET << std::endl;
	}
	else if (this->sending_level == SENDING_END) {
		std::cout << CYAN << "sending_level = SENDING_END" << RESET << std::endl;
	}
	if (this->method_level == FINDRESOURCE) {
		std::cout << CYAN << "method_level = FINDRESOURCE" << RESET << std::endl;
	}
	else if (this->method_level == DATA_SENDING) {
		std::cout << CYAN << "method_level = DATA_SENDING" << RESET << std::endl;
	}
	else if (this->method_level == DATA_SEND) {
		std::cout << CYAN << "method_level = DATA_SEND" << RESET << std::endl;
	}
	if (this->request_case == OTHER_CASE) {
		std::cout << CYAN << "request_case = OTHER_CASE" << RESET << std::endl;
	}
	else if (this->request_case == DIR_CASE) {
		std::cout << CYAN << "request_case = DIR_CASE" << RESET << std::endl;
	}
	else if (this->request_case == FILE_CASE) {
		std::cout << CYAN << "request_case = FILE_CASE" << RESET << std::endl;
	}
	else if (this->request_case == NO_CASE) {
		std::cout << CYAN << "request_case = NO_CASE" << RESET << std::endl;
	}
	if (this->match_index == YES) {
		std::cout << CYAN << "match_index = YES" << RESET << std::endl;
	}
	else if (this->match_index == NO) {
		std::cout << CYAN << "match_index = NO"  << RESET << std::endl;
	}
}
