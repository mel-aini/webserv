#include "Response.hpp"

Response::Response() : status(200) {}
Response::~Response() {}

void	Response::setStatus(unsigned int status) {
	this->status = status;
}

void	Response::setLocation(Location *location) {
	this->location = location;
}

Location    *Response::getLocation() {
	return this->location;
}


void	Response::send_4xxResponse(unsigned int status)
{
	(void)status;
	/*
		if (status == 400)
			then: a 400 bad request response
		if (status == 405)
			then: a 405 Method Not Allowed response
	*/
}

void	Response::send_status_line_and_headers(int fd)
{
	// title: prepare status line
	std::stringstream str;
	str << this->status;
	std::string status = str.str();
	std::string status_line = "HTTP/1.1 " + status + " " + this->message + '\n';

	std::string headers;
	std::map<std::string, std::string>::iterator it;
	for (it = this->headers.begin(); it != this->headers.end(); it++) {
		headers += it->first;
		headers += it->second;
		headers += '\n';
	}

	std::string response = status_line + headers + "\r\n\r\n";
	std::cout << YELLOW << response << RESET << std::endl;
	const char *buf = response.c_str();
	if (send(fd, buf, response.size(), 0) == -1)
		throw ResponseFailed();
}

void    Response::redirect(int fd, const std::string& location)
{
	// then: send a 301 Moved Permanently with uri in <Location> header
	this->status = 301;
	this->message = "Moved Permanently";
	this->headers["Location: "] = location;
	send_status_line_and_headers(fd);
}

bool compareByLength(Location& a, Location& b)
{
    return (a.getPath().length() > b.getPath().length());
}

Location *Response::findLocation(std::vector<Location> &locations, std::string uri)
{
	std::vector<Location>::iterator	it;
	std::string tmp = uri;

	if (locations.size() > 1)
		std::sort(locations.begin(), locations.end(), compareByLength);
	if (tmp[tmp.length() - 1] == '/')
	{
		if (tmp[tmp.length() - 2] == '/')
			return NULL;
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

// title: exceptions

const char	*Response::ResponseFailed::what() const throw() {
	return "Error occured while sending response";
}
