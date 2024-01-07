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
	sendingFile(false)
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
	std::cout << "send: " << BOLDCYAN << static_cast<double>(bodyOffset) / 1000000 << "Mb" << RESET << std::endl;
	if (file.eof()) {
		this->sending_level = SENDING_END;
		std::cout << "bytes sent: " << BOLDGREEN << static_cast<double>(bodyOffset) / 1000000 << "Mb" << RESET << std::endl;
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
	std::cout << "REDIRECT" << std::endl;
}

bool	compareByLength(Location& a, Location& b)
{
    return (a.getPath().length() > b.getPath().length());
}

Location *Response::findLocation(std::vector<Location> &locations, std::string uri)
{
	std::vector<Location>::iterator	it;
	std::string tmp = uri;

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

// bool	hasQueryString(std::string uri)
// {
// 	size_t	i;
// 	for (i = 0; i < uri.length(); i++)
// 		if (uri[i] == '?')
// 			break ;
// 	if (i == uri.length())
// 		return (false);
// 	return (true);
// }

/*
bool	Response::getRequestedResource(std::string uri)
{
	memset(&this->fileInf, 0, sizeof(this->fileInf));
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
	std::string	fileCase = this->location->getRoot() + "/" + uri;
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
*/

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

	if (!this->isFileExist(target)) {
		throw 404;
	}

	struct stat fileInfo;

	if (this->isTarget(target, &fileInfo)) {
		// std::cout << BOLDRED << "isTarget" << RESET << std::endl;
		return true;
	}
	else if (S_ISDIR(fileInfo.st_mode)) {
		std::vector<std::string>::iterator it;
		target += "/";
		for (it = this->location->getIndex().begin(); it != this->location->getIndex().end(); it++) {
			target += *it;
			// std::cout << "2nd target: " << target << std::endl;
			if (this->isFileExist(target)) {
				struct stat fileInfo2;
				if (!this->isTarget(target, &fileInfo2))
					throw 403;
				return true;
			}
		}
		// std::cout << BOLDRED << "No Index" << RESET << std::endl;
		return false;
	}
	std::cout << "file to send: " << "[" + this->fileToSend + "]" << std::endl;
	return false;
}

// ... working on
bool	Response::newGet(std::string uri) {
	// todo: new GET
	if (this->sending_level == GET_REQUESTED_RES) {
		if (getRequestedFile(uri))
			this->sending_level = SENDING_HEADERS;
		else {
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

/*
std::pair<std::string, size_t>	Response::getMatchIndex()
{
	std::pair<std::string, size_t>	pair;
	std::vector<std::string>	index = this->location->getIndex();
	std::vector<std::string>::iterator	it = index.begin();

	std::string	path;
	if (uri[0] != '/')
		path = this->location->getRoot() + "/" + uri;
	else
		path = this->location->getRoot() + uri;
	if (uri[uri.length() - 1] != '/')
		path += "/";
	struct stat	dirCaseInf;
	for (; it != index.end(); it++)
	{
		pair.first = path + *it;
		if (stat(pair.first.c_str(), &dirCaseInf) == 0)
			if (S_ISREG(dirCaseInf.st_mode))
				break ;
	}
	if (it == index.end())
	{
		this->match_index = NO;
		return (pair);
	}
	pair.second = dirCaseInf.st_size;
	this->match_index = YES;
	return (pair);
}
*/

std::string	Response::getContentType(std::string path)
{
	std::string	extension = path.substr(path.rfind('.'));
	if (this->content_type[extension].empty())
		return ("");
	return (this->content_type[extension]);
}

bool	Response::readAndSendFile(std::string path, size_t size)
{
	if (this->sending_level == SENDING_HEADERS)
	{
		// std::cout << "here" << std::endl;
		std::stringstream sizestream;
		sizestream << size;
		this->headers["Content-Type: "] = getContentType(path);
		this->headers["Content-Lenght: "] = sizestream.str();
		this->send_status_line_and_headers();
		this->sending_level = SENDING_BODY;
	}
	else if (this->sending_level == SENDING_BODY) {
		if (this->sendFile(path))
			this->sending_level = SENDING_END;
	}
	else if (this->sending_level == SENDING_END)
		return true;
	return (false);
}
/*
bool	Response::getMethod(std::string uri)
{
	// log();
	if (this->method_level == FINDRESOURCE)
	{
		if (this->getRequestedResource(uri)) {
			this->method_level = DATA_SENDING;
		}
		else
		{
			this->status = 404;
			this->response_type = ERROR;
			return (false);
		}
	}
	if (this->method_level == DATA_SENDING)
	{
		if (this->request_case == FILE_CASE)
		{
			if (env)
			{
				int	fd[2];
				if (pipe(fd) == -1)
				{
					std::cerr << "pipe() fail" << std::endl; // generate a error message
					std::exit(EXIT_FAILURE);
				}
				pid_t	pid = fork();
				if (pid == -1)
				{
					std::cerr << "fork() fail" << std::endl; // generate a error message
					std::exit(EXIT_FAILURE);
				}
				if (pid == 0)
				{
					dup2(fd[1], 1);
					close(fd[0]);
					close(fd[1]);
					execve("cgi/php-cgi", NULL, env);
				}
				wait(0);
				dup2(fd[0], 0);
				close(fd[0]);
				close(fd[1]);
				std::ofstream file("file");
				std::string str;
				if (file.is_open())
					while (getline(std::cin, str))
						file << str << '\n';
				file.close();
				exit(0);
			}
			else
			{
				std::string	fileCase;
				if (uri[0] != '/')
					fileCase = this->location->getRoot() + "/" + uri;
				else
					fileCase = this->location->getRoot() + uri;
				if (this->readAndSendFile(fileCase, fileInf.st_size))
					this->method_level = DATA_SEND;
			}
		}
		else if (this->request_case == OTHER_CASE) {
			std::cout << "OTHER_CASE" << std::endl;
			if (uri[0] == '/')
				uri.erase(0, 1);
			if (this->readAndSendFile(uri, fileInf.st_size))
				this->method_level = DATA_SEND;
		}
		else if (this->request_case == DIR_CASE)
		{
			std::cout << "DIR_CASE" << std::endl;
			if (uri[uri.length() - 1] != '/')
			{
				redirect(uri + "/");
				this->method_level = DATA_SEND;
			}
			else
			{
				if (this->location->getIndex().size() != 0)
				{
					std::pair<std::string, size_t>	pair = this->getMatchIndex(uri);
					if (env)
					{
						int	fd[2];
						if (pipe(fd) == -1)
						{
							std::cerr << "pipe() fail" << std::endl; // generate a error message
							std::exit(EXIT_FAILURE);
						}
						pid_t	pid = fork();
						if (pid == -1)
						{
							std::cerr << "fork() fail" << std::endl; // generate a error message
							std::exit(EXIT_FAILURE);
						}
						if (pid == 0)
						{
							dup2(fd[1], 1);
							close(fd[0]);
							close(fd[1]);
							execve("cgi/php-cgi", NULL, env);
						}
						wait(0);
						dup2(fd[0], 0);
						close(fd[0]);
						close(fd[1]);
						std::ofstream file;
						file.open("file");
						std::string str;
						while (getline(std::cin, str))
							file << str << "\n";
						exit(0);
					}
					else
					{
						std::cout << pair.first << std::endl;
						if (this->match_index == YES)
						{
							if (this->readAndSendFile(pair.first, pair.second))
								this->method_level = DATA_SEND;
						}
						else
						{
							this->status = 404;
							this->response_type = ERROR;
							return (false);
						}
					}
				}
				else
				{
					if (this->location->getAutoIndex())
					{
						DIR *dir = opendir(uri.erase(0, 1).c_str());
						if (dir)
						{
							struct dirent *dirContent;
							std::vector<std::string> content;
							while ((dirContent = readdir(dir)) != NULL)
								content.push_back(dirContent->d_name);
							// if (this->send_response_index_files(uri, content))
							// 	this->method_level = DATA_SEND;
						}
						return (true);
					}
					else
					{
						this->status = 403;
						this->response_type = ERROR;
						return (false);
					}
				}
			}
		}
		else
		{
			this->status = 404;
			this->response_type = ERROR;
			return (false);
		}
	}
	if (this->method_level == DATA_SEND) {
		return (true);
	}
	return (false);
}
*/
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
	memset(&this->fileInf, 0, sizeof(this->fileInf));
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