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