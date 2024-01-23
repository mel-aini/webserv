#include "Response.hpp"

Response::Response() 
	:
	status(200), 
	location(NULL),
	sending_level(GET_REQUESTED_RES),
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
	status_codes[409] = "Conflict";
	status_codes[411] = "Length Required";
	status_codes[413] = "Payload Too Large";
	status_codes[414] = "URI Too Long";
	status_codes[500] = "Internal Server Error";
	status_codes[501] = "Not Implemented";
	status_codes[502] = "Bad Gateway";
	status_codes[505] = "HTTP Version Not Supported";
	status_codes[415] = "Unsupported Media Type";


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

Response::~Response() {}

// title: Getters

Log&				Response::getTraces() {
	return this->traces;
}

int					Response::getSocket()
{
	return (this->socket);
}

int					Response::getStatus() const {
	return this->status;
}

unsigned int		Response::getResponseType() const {
	return this->response_type;
}

unsigned int	    Response::getSendingLevel() const {
	return this->sending_level;
}

Location    		*Response::getLocation() {
	return this->location;
}

std::string 		Response::getStatusMessage() {
	return this->status_codes[this->status];
}

// title: Setters

void	Response::setStatus(unsigned int status) {
	this->status = status;
}

void	Response::setLocation(Location *location) {
	this->location = location;
}

void    Response::setSendingLevel(unsigned int level) {
	this->sending_level = level;
}

void	Response::setResponseType(unsigned int response_type)
{
	this->response_type = response_type;
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
	char buf[4000] = {0};

	std::ifstream file(fileName.c_str(), std::ios::binary | std::ios::in);
	if (!file.is_open()) {
		// std::cout << BOLDRED << "[ERROR] : !FILE.IS_OPEN()" << RESET << std::endl;
		throw ConnectionClosed();
	}

	file.seekg(this->bodyOffset, std::ios::beg);

	file.read(buf, sizeof(buf));
	int bytesRead = file.gcount();
	int s = send(this->socket, buf, bytesRead, 0);
	if (s <= 0) {
		file.close();
		// std::cout << BOLDRED << "[ERROR] : SEND <= 0" << RESET << std::endl;
		throw ConnectionClosed();
	}

	bodyOffset += s;

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
	if (this->sending_level == GET_REQUESTED_RES)
		this->sending_level = SENDING_HEADERS;
	if (this->sending_level == SENDING_HEADERS)
	{
		std::stringstream sizestream;
		if (this->location && this->isInErrorPages())
		{
			std::cout << "is in error pages" << std::endl;
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
			std::cout << "is not in error pages" << std::endl;
			std::cout << this->status << std::endl;
			std::string message = this->getStatusMessage();
			HtmlTemplate htmlErrorPage(this->status, message);
			sizestream << htmlErrorPage.getHtml().size();
		}
		this->headers["Content-Type: "] = "text/html";
		this->headers["Content-Length: "] = sizestream.str();

		send_status_line_and_headers();
		this->sending_level = SENDING_BODY;
	}
	else if (this->sending_level == SENDING_BODY)
	{
		if (this->sendingFile) {
			return this->sendFile(this->location->root + "/" + this->errPage);
		}
		else {
			std::string message = this->getStatusMessage();
			HtmlTemplate htmlErrorPage(this->status, message);

			const std::string& response = htmlErrorPage.getHtml();

			const char *buf = response.c_str();
			if (send(this->socket, buf, response.size(), 0) <= 0)
				throw ConnectionClosed();

			this->sending_level = SENDING_END;
		}
	}
	return this->sending_level == SENDING_END;
}

bool	Response::send_response_index_files(std::string uri)
{
	if (uri[0] == '/')
		uri.erase(0, 1);

	std::string	target = this->location->getRoot() + uri;

	DIR *dir = opendir(target.c_str());
	if (!dir)
		throw 404;

	struct dirent *dirContent;
	std::vector<std::string> content;
	while ((dirContent = readdir(dir)) != NULL)
		content.push_back(dirContent->d_name);
	closedir(dir);
	std::stringstream	sizestream;
	HtmlTemplate htmlIndex(target, content);
	sizestream << htmlIndex.getHtml().size();
	this->headers["Content-Type: "] = "text/html";
	this->headers["Content-Length: "] = sizestream.str();

	send_status_line_and_headers();

	const std::string& response = htmlIndex.getHtml();

	const char *buf = response.c_str();
	if (send(this->socket, buf, response.size(), 0) <= 0)
		throw ConnectionClosed();

	this->sending_level = SENDING_END;
	return true;
}

void	Response::send_status_line_and_headers()
{
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
	std::cout << response << std::endl;
	const char *buf = response.c_str();
	if (send(this->socket, buf, response.size(), 0) <= 0)
		throw ConnectionClosed();	
}

void    Response::redirect(const std::string& location)
{
	// then: send a 301 Moved Permanently with uri in <Location> header
	this->status = 301;
	this->headers["Location: "] = location;
	send_status_line_and_headers();
}

bool	Response::isFileExist(std::string& target) {
	if (access(target.c_str(), F_OK) == 0) { // then: exist 
		if (access(target.c_str(), W_OK) == 0) { // then: has permission
			return true;
		}
		else {
			throw 403;
		}
	}
	return false;
}

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

void	Response::decode_uri(std::string& uri)
{
	size_t l = 0;
	for (; l < uri.length(); l++)
	{
		if (uri[l] == '+')
			uri[l] = ' ';
		else if (uri[l] == '%' && uri[l + 1] && uri[l + 2])
		{
			std::string hexChar = uri.substr(l + 1, 2);
			std::stringstream ss(hexChar);
			int c;
			ss >> std::hex >> c;
			// std::cout << l << std::endl;
			if (c != 0)
			    uri = uri.substr(0, l) + static_cast<char>(c) + uri.substr(l + 3);
		}
	}
}

bool	Response::getRequestedResource(std::string uri)
{
	size_t qsPos = uri.find('?');
	uri = (qsPos != std::string::npos) ? uri.substr(0, qsPos) : uri;
	if (uri[0] == '/')
		uri.erase(0, 1);

	std::string	target = this->location->getRoot() + uri;

	if (!this->isFileExist(target)) {
		throw 404;
	}

	struct stat fileInfo;

	if (this->isTarget(target, &fileInfo)) {
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

		if (index.size() == 0)
			return false;
	
		for (it = index.begin(); it != index.end(); it++) {
			std::string target2 = target + *it;

		
			if (this->isFileExist(target2)) {
				struct stat fileInfo2;
				if (!this->isTarget(target2, &fileInfo2)) {
					throw 403;
				}
				return true;
			}
		}
		throw 404;
	}
	return false;
}

bool	Response::hasCgi(void)
{
	if (this->location->getCgiExec().size() == 0)
		return (false);
	std::vector<std::pair<std::string, std::string> > cgiExec = this->location->getCgiExec();
	std::vector<std::pair<std::string, std::string> >::iterator it = cgiExec.begin();
	for (; it != cgiExec.end(); it++)
	{
		size_t ptPos = this->fileToSend.rfind('.');
		if (ptPos != std::string::npos && it->second == this->fileToSend.substr(ptPos))
		{
			this->matchCgi = *it;
			return (true);
		}
	}
	return (false);
}

bool	Response::post_method(Request &request, std::map <std::string, std::string> firstCgiEnv) {
	if (location->getAcceptUpload()) {
		return this->uploadPostMethod(request);
	}
	if (this->sending_level == GET_REQUESTED_RES) {
		bool isIndex = getRequestedResource(request.getUri());
		if (sending_level == SENDING_END)
			return true;
		if (!isIndex)
			throw (403);
		else
			this->sending_level = SENDING_HEADERS;
	}
	else if (this->sending_level == SENDING_HEADERS) {
		if (!this->hasCgi())
			throw (403);
		this->cgi.executeCgi(this->fileToSend ,this->matchCgi.first, request.getFilename(), firstCgiEnv, POST);
		this->cgi.sendCgiHeader(this->socket);
		this->sending_level = SENDING_BODY;
	}
	else if (this->sending_level == SENDING_BODY) {
		if (this->cgi.sendCgiBody(this->socket))
			this->sending_level = SENDING_END;
	}
	return this->sending_level == SENDING_END;
}

void	Response::check_dir_permission(std::string target) {
	if (access(target.c_str(), W_OK) != 0) {
		throw 403;
	}

	DIR *dir = opendir(target.c_str());
	if (!dir)
		throw 404;
	std::string oldTarget = target;
	struct dirent *dirContent;
	struct stat fileInfo;

	while ((dirContent = readdir(dir)) != NULL) {
		std::string	dirstring = dirContent->d_name;
		if (dirstring == "." || dirstring == "..") {
			continue;
		}
		std::string newTarget = oldTarget + "/" + dirstring;
		if (stat(newTarget.c_str(), &fileInfo) == 0) {
			if (access(newTarget.c_str(), W_OK) != 0) {
				closedir(dir);
				throw 403;
			}
			if (S_ISDIR(fileInfo.st_mode)) {
				check_dir_permission(newTarget);
			}
		} else {
			closedir(dir);
			throw 404;
		}
	}
	closedir(dir);
}

void	Response::remove_dir(std::string target) {
	DIR *dir = opendir(target.c_str());
	if (!dir)
		throw 404;

	std::string oldTarget = target;
	struct dirent *dirContent;
	struct stat fileInfo;

	while ((dirContent = readdir(dir)) != NULL) {
		std::string	dirstring = dirContent->d_name;
		if (dirstring == "." || dirstring == "..") {
			continue;
		}
		std::string newTarget = oldTarget + "/" + dirstring;
		if (stat(newTarget.c_str(), &fileInfo) == 0) {
			if (S_ISREG(fileInfo.st_mode)) {
				if (unlink(newTarget.c_str()) == -1) {
					closedir(dir);
					throw 500;
				}
			}
			else if (S_ISDIR(fileInfo.st_mode)) {
				remove_dir(newTarget);
			}
		} else {
			closedir(dir);
			throw 404;
		}
	}
	if (rmdir(target.c_str()) == -1) {
		closedir(dir);
		throw 500;
	}
	closedir(dir);
}

bool	Response::delete_method(std::string uri) {
	if (uri[0] == '/')
		uri.erase(0, 1);

	decode_uri(uri);

	std::string	target = this->location->getRoot() + uri;
	this->fileToSend = target;

	if (!isFileExist(target))
		throw 404;
	
	struct stat fileInfo;

	if (stat(target.c_str(), &fileInfo) != 0)
		throw 404;

	if (S_ISREG(fileInfo.st_mode)) {
		if (access(target.c_str(), W_OK) != 0)
			throw 403;

		if (unlink(target.c_str()) == -1)
			throw 500;
	}
	else if (S_ISDIR(fileInfo.st_mode)) {
		if (uri.back() != '/')
			throw 409;

		check_dir_permission(target);
		remove_dir(target);
	}
	this->setStatus(204);
	this->headers["Content-Length: "] = "0";
	send_status_line_and_headers();
	return true;
}

bool	Response::get_method(std::string uri, std::map <std::string, std::string> firstCgiEnv, std::string bodyFileName) {
	if (this->sending_level == GET_REQUESTED_RES) {
		bool isNoIndex = getRequestedResource(uri);
		if (sending_level == SENDING_END)
			return true;

		if (isNoIndex)
			this->sending_level = SENDING_HEADERS;

		if (!isNoIndex) {
			// then: autoIndex
			if (!this->location->getAutoIndex())
				throw 403;
			send_response_index_files(uri);
			this->sending_level = SENDING_END;
		}
	}
	if (this->sending_level == SENDING_HEADERS) {
		if (this->hasCgi())
		{
			this->cgi.executeCgi(this->fileToSend ,this->matchCgi.first, bodyFileName, firstCgiEnv, GET);
			this->cgi.sendCgiHeader(this->socket);
		}
		else
		{
			std::ifstream file(this->fileToSend.c_str(), std::ios::binary | std::ios::in);
			if (!file.is_open())
				throw 404;

			std::stringstream sizestream;
			struct stat fileInfo;
			if (stat(this->fileToSend.c_str(), &fileInfo) != 0) {
				file.close();
				throw 404;
			}

			sizestream << fileInfo.st_size;
			this->headers["Content-Length: "] = sizestream.str();
			if (fileInfo.st_size != 0)
				this->headers["Content-Type: "] = getContentType(this->fileToSend);

			send_status_line_and_headers();
			if (fileInfo.st_size == 0) {
				this->sending_level = SENDING_END;
				return true;
			}
			file.close();
		}
		this->sending_level = SENDING_BODY;
	}
	else if (this->sending_level == SENDING_BODY) {
		if (this->hasCgi()) {
			if (this->cgi.sendCgiBody(this->socket))
				this->sending_level = SENDING_END;
		}
		else {
			return this->sendFile(this->fileToSend);
		}
	}
	return this->sending_level == SENDING_END;
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

const char	*Response::ConnectionClosed::what() const throw() {
	return "Client Closed it's endpoint";
}

// title: log methods

void    Response::log_members() {
	std::stringstream s1;
	s1 << sending_level;

	std::stringstream s2;
	s2 << response_type;

	std::stringstream s3;
	s3 << status;
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

void    Response::log_response() {
	// std::cout << BOLDGREEN << "[DONE][" << this->socket << "]: http response: # " << RESET;
	std::cout << "HTTP/1.1 " << this->status << " " + this->status_codes[this->status] << " | ";

	std::string headers;
	std::map<std::string, std::string>::iterator it = this->headers.begin();
	for (; it != this->headers.end(); it++) {
		std::cout << it->first << it->second << " *|* ";
	}
	std::cout << std::endl;
}
