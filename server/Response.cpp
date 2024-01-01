#include "Response.hpp"
#include <fcntl.h>

Response::Response() 
	:
	status(200), 
	location(NULL),
	sending_level(SENDING_HEADERS),
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
}

Response::~Response() {}

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

bool	Response::sendFile(std::string fileName)
{
	char buf[1024] = {0};

	std::ifstream file(fileName.c_str(), std::ios::binary | std::ios::in);
	if (!file.is_open()) {
		std::cerr << BOLDRED << "Error: Unable to open infile" << RESET << std::endl;
		throw ResponseFailed();
	}
	file.seekg(this->bodyOffset, std::ios::beg);
	if (!file || file.eof())
		this->sending_level = SENDING_END;

	file.read(buf, sizeof(buf));
	if (!file)
		this->sending_level = SENDING_END;

	int bytesRead = file.gcount();
	bodyOffset += bytesRead;

	if (bytesRead != 0)
		send(this->socket, buf, bytesRead, 0);

	// std::cout << RED << buf << RESET << std::endl;

	if (file.eof()) {
		this->sending_level = SENDING_END;
		file.close();
		return true;
	}
	file.close();
	return false;
}

bool	Response::send_response_error()
{
	// std::cout << MAGENTA << "Here!" << RESET << std::endl;
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
			std::cout << RED << response << RESET << std::endl;
			std::cout << RED << "size: "<< response.size() << RESET << std::endl;

			const char *buf = response.c_str();
			if (send(this->socket, buf, response.size(), 0) == -1)
				throw ResponseFailed();

			this->sending_level = SENDING_END;
		}
	}
	else if (this->sending_level == SENDING_END)
	{
		this->reset();
		return true;
	}
	/*
		if (this error in error pages) {
			get error page
			if (found) {
				read from it
				return
			}
		}
		if (status == 400)
			then: a 400 Bad Request response
			-> generate an html response template(400, "Bad Request")
		else if (status == 403)
			then: a 403 Forbidden response
			-> generate an html response template(403, "Forbidden")
		else if (status == 404)
			then: a 404 Not Found response
			-> generate an html response template(404, "Not Found")
		else if (status == 405)
			then: a 405 Method Not Allowed response
			-> generate an html response template(405, "Method Not Allowed")
		}
	*/
	// return true;
	return false;
}
bool	Response::send_response_index_files(std::string path, std::vector<std::string> content)
{
	// std::cout << MAGENTA << "Here!" << RESET << std::endl;
	if (this->sending_level == SENDING_HEADERS)
	{
		std::stringstream	sizestream;
		HtmlTemplate htmlErrorPage(path, content);
		sizestream << htmlErrorPage.getHtml().size();
		this->headers["Content-Type: "] = "text/html";
		this->headers["Content-Length: "] = sizestream.str();

		send_status_line_and_headers();
		this->sending_level = SENDING_BODY;
	}
	if (this->sending_level == SENDING_BODY)
	{
		HtmlTemplate htmlErrorPage(path, content);

		const std::string& response = htmlErrorPage.getHtml();
		std::cout << RED << response << RESET << std::endl;
		std::cout << RED << "size: "<< response.size() << RESET << std::endl;

		const char *buf = response.c_str();
		if (send(this->socket, buf, response.size(), 0) == -1)
			throw ResponseFailed();

		this->sending_level = SENDING_END;
	}
	else if (this->sending_level == SENDING_END)
	{
		this->reset();
		return true;
	}
	return false;
}

void	Response::send_status_line_and_headers()
{
	// title: prepare status line
	std::stringstream str;
	str << this->status;
	std::string status = str.str();
	this->message = this->status_codes[this->status];
	std::string status_line = "HTTP/1.1 " + status + " " + this->message + "\n";

	std::string headers;
	std::map<std::string, std::string>::iterator it = this->headers.begin();
	while (it != this->headers.end()) {
		headers += it->first;
		headers += it->second;
		it++;
		if (it != this->headers.end())
			headers += '\n';
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
}

void	Response::reset() {
	this->status = 200;
	this->sending_level = SENDING_HEADERS;
	this->method_level = FINDRESOURCE;
	this->request_case = NO_CASE;
	this->match_index = NO;
	this->headers.clear();
	this->body = "";
	this->location = NULL;
	this->bodyOffset = 0;
}

bool compareByLength(Location& a, Location& b)
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

// std::vector<std::string>	parseQueryString(std::string uri)
// {
//     std::vector<std::string>	queryString;
// 	size_t i;
// 	for (i = 0; i < uri.length(); i++)
// 		if (uri[i] == '?')
// 		{
// 		    i++;
// 			break ;
// 		}
// 	if (i == uri.length())
// 		return (queryString);
// 	size_t j = i;
// 	for (; j < uri.length(); j++)
// 	{
// 		if (uri[j] == '&')
// 		{
// 		    j++;
// 			std::string	tmp = uri.substr(i, j - i - 1);
// 			size_t l = 0;
// 			for (; l < tmp.length(); l++)
// 			{
// 				if (tmp[l] == '+')
// 					tmp[l] = ' ';
// 				else if (tmp[l] == '%')
// 				{
// 					std::string hexChar = tmp.substr(l + 1, 2);
// 					std::stringstream ss(hexChar);
// 					int c;
// 					ss >> std::hex >> c;
// 					tmp = tmp.substr(0, l) + static_cast<char>(c) + tmp.substr(l + 3);
// 				}
// 			}
// 			queryString.push_back(tmp);
// 			i = j;
// 		}
// 	}
// 	std::string	tmp = uri.substr(i, j - i);
// 	size_t l = 0;
// 	for (; l < tmp.length(); l++)
// 	{
// 		if (tmp[l] == '+')
// 			tmp[l] = ' ';
// 		else if (tmp[l] == '%')
// 		{
// 			std::string hexChar = tmp.substr(l + 1, 2);
// 			std::stringstream ss(hexChar);
// 			int c;
// 			ss >> std::hex >> c;
// 			tmp = tmp.substr(0, l) + static_cast<char>(c) + tmp.substr(l + 3);
// 		}
// 	}
// 	queryString.push_back(tmp);
// 	return (queryString);
// }

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

bool	Response::getRequestedResource(std::string uri)
{
	if (uri[0] == '/')
		uri.erase(0, 1);
	if (stat(uri.c_str(), &this->fileInf) == 0)
	{
		if (S_ISREG(this->fileInf.st_mode)){
			this->request_case = OTHER_CASE;
		}
		return (true);
	}

	std::string	fileCase = this->location->getRoot() + this->location->getPath();
	if (stat(fileCase.c_str(), &this->fileInf) == 0)
	{
		if (S_ISREG(this->fileInf.st_mode))
			this->request_case = FILE_CASE;
		return (true);
	}

	if (stat(this->location->getRoot().c_str(), &this->fileInf) == 0)
	{
		if (S_ISDIR(this->fileInf.st_mode))
			this->request_case = DIR_CASE;
		return (true);
	}
	return (false);
}

std::pair<std::string, size_t>	Response::getMatchIndex()
{
	std::pair<std::string, size_t>	pair;
	std::vector<std::string>	index = this->location->getIndex();
	std::vector<std::string>::iterator	it = index.begin();

	struct stat	dirCaseInf;
	for (; it != index.end(); it++)
	{
		pair.first = this->location->getRoot() + "/" + *it;
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

std::string	getContentType(std::string path)
{
	std::string	extension = path.substr(path.rfind('.'));
	if (extension == ".html" || extension == ".htm")
		return ("text/html");
	if (extension == ".jpeg" || extension == ".jpg")
		return ("image/jpeg");
	if (extension == ".mp3")
		return ("audio/mpeg");
	if (extension == ".mp4")
		return ("video/mp4");
	if (extension == ".png")
		return ("image/png");
	return (0);
}

bool	Response::readAndSendFile(std::string path, size_t size)
{
	if (this->sending_level == SENDING_HEADERS)
	{
		std::stringstream sizestream;
		sizestream << size;
		this->headers["Content-Type: "] = getContentType(path);
		this->headers["Content-Lenght: "] = sizestream.str();
		this->send_status_line_and_headers();
		this->sending_level = SENDING_BODY;
	}
	else if (this->sending_level == SENDING_BODY)
	{
		if (this->sendFile(path))
			this->sending_level = SENDING_END;
		return (false);
	}
	else if (this->sending_level == SENDING_END)
		return true;
	return (false);
}

bool	Response::getMethod(std::string uri)
{
	std::cout << "uri: " + uri << std::endl;
	if (this->method_level == FINDRESOURCE)
	{
		if (this->getRequestedResource(uri)){
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
			if (this->location->getCgiExec().size() != 0)
			{
				// int	fd[2];
				// if (pipe(fd) == -1)
				// {
				// 	std::cerr << "pipe() fail" << std::endl; // generate a error message
				// 	std::exit(EXIT_FAILURE);
				// }
				// pid_t	pid = fork();
				// if (pid == -1)
				// {
				// 	std::cerr << "fork() fail" << std::endl; // generate a error message
				// 	std::exit(EXIT_FAILURE);
				// }
				// if (pid == 0)
				// {
				// 	dup2(fd[1], 1);
				// 	close(fd[0]);
				// 	close(fd[1]);
				// 	char* arg[2] = {const_cast<char *>(fileCase.c_str()), NULL};
				// 	// char** env = getCgiEnv(uri);
				// 	if (hasQueryString(uri))
				// 	{
				// 		std::string queryString = uri.substr(uri.find('?') + 1);
				// 		char* env[3] = {const_cast<char *>("REQUEST_METHOD=GET"), const_cast<char *>(queryString.c_str()), NULL};
				// 		execve("cgi/php-cgi", arg, env);
				// 		std::cerr << "execve() fail" << std::endl; // generate a error message
				// 		std::exit(EXIT_FAILURE);
				// 	}
				// 	// else
				// 		char* env[2] = {const_cast<char *>("REQUEST_METHOD=GET"), NULL};
				// 		execve("cgi/php-cgi", arg, env);
				// 		std::cerr << "execve() fail" << std::endl; // generate a error message
				// 		std::exit(EXIT_FAILURE);
				// }
				// wait(0);
			}
			else
			{
				std::string	fileCase = this->location->getRoot() + this->location->getPath();
				return (this->readAndSendFile(fileCase, fileInf.st_size));
			}
		}
		else if (this->request_case == OTHER_CASE) {
			return (this->readAndSendFile(uri, fileInf.st_size));
		}
		else if (this->request_case == DIR_CASE)
		{
			if (uri[uri.length() - 1] != '/')
			{
				redirect(uri + "/");
				return (true);
			}
			else
			{
				if (this->location->getIndex().size() != 0)
				{
					std::pair<std::string, size_t>	pair = this->getMatchIndex();
					if (this->location->getCgiExec().size() != 0)
					{}
					else
					{
						if (this->match_index == YES)
							return (this->readAndSendFile(pair.first, pair.second));
						// else
						// {
						// 	this->status = 404;
						// 	this->response_type = ERROR;
						// 	return (false);
						// }
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
							if (this->send_response_index_files(uri, content))
								return (true);
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
		{std::cout << "here\n";
			this->status = 404;
			this->response_type = ERROR;
			return (false);
		}
	}
	// std::cout << "here\n";
	return (false);
}

// title: exceptions

const char	*Response::ResponseFailed::what() const throw() {
	return "Error occured while sending response";
}
