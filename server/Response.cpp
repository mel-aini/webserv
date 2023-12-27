#include "Response.hpp"

Response::Response() : status(200) {}

void	Response::setStatus(unsigned int status) {
	this->status = status;
}

Response::~Response() {}

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

void    Response::redirect(std::string uri) {
	(void)uri;
	// then: send a 301 Moved Permanently with uri in <Location> header
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
