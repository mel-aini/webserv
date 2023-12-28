#include "Response.hpp"

Response::Response() 
	:
	status(200), 
	location(NULL),
	sending_level(SENDING_HEADERS),
	response_type(OK),
	bodyOffset(0)
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

bool	Response::isInErrorPages(std::string& errPage)
{
	std::vector<std::pair<std::string, std::vector<int> > >::iterator	it;

	for (it = location->errorPages.begin(); it != location->errorPages.end(); it++) {
		std::vector<int>::iterator it2;
		for (it2 = it->second.begin(); it2 != it->second.end(); it2++) {
			if (this->status == (unsigned int)*it2) {
				errPage = it->first;
				return true;
			}
		}
	}
	return false;
}

bool	Response::send_response_error()
{
	std::cout << MAGENTA << "Here!" << RESET << std::endl;
	if (this->sending_level == SENDING_HEADERS)
	{
		/*
			switch (sending_level)
			{
				case SENDING_HEADERS:
					if (sending a file) {
						if (file exist)
							get content-length from file;
							sending a file = false
						else if (file not exist or failed)
							get content-length from html template
						...
					}
					add
			}
		*/
		std::string message = this->getStatusMessage();
		HtmlTemplate htmlErrorPage(this->status, message);
		std::stringstream ss;
		ss << htmlErrorPage.getHtml().size();

		this->headers["Content-Type: "] = "text/html";
		this->headers["Content-Length: "] = ss.str();

		send_status_line_and_headers();
		this->sending_level = SENDING_BODY;
	}
	if (this->sending_level == SENDING_BODY)
	{
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
	std::string errPage;

	if (this->location && this->isInErrorPages(errPage))
	{
		char buf[1024] = {0};
		std::string fileName = this->location->root + "/" + errPage;
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

		std::cout << RED << buf << RESET << std::endl;

		if (file.eof()) {
			file.close();
			this->sending_level = SENDING_END;
			return true;
		}
		return false;
	}
	std::string message = this->getStatusMessage();
	HtmlTemplate htmlErrorPage(this->status, message);
	return true;
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
	std::cout << YELLOW << response << RESET << std::endl; 
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
	std::cout << YELLOW << it->getPath() << RESET << std::endl;
	return &(*it);
}

// title: exceptions

const char	*Response::ResponseFailed::what() const throw() {
	return "Error occured while sending response";
}
