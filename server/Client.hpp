#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include "../public/Colors.hpp"
#include <poll.h>
#include <sstream>
#include <cstring>
#include <string>
#include <fstream>
#include "Request.hpp"
#include "Response.hpp"

enum read_state {
	REQUEST_LINE,
	HEADERS,
	BODY,
	EOR,
};

class Client
{
	private:
		int					fd;
		struct sockaddr_in	address;
		bool				resIsSent;
		bool				reqIsArrived;
		bool				firstInteraction;
		struct pollfd		*pollfd;
		Request				request;
		Response			response;
		

	public:
		Client(int fd, struct sockaddr_in address);
		~Client();

		int					getFd() const;
		struct sockaddr_in	getAddress() const;
		void				log();
		void				readRequest(struct pollfd *pollfd);
		void				createResponse(std::string host);
		void				reqHasRead();
		void				resHasSent();
		void				reset();
		void				setPollfd(struct pollfd	*pollfd);
		void				getMethod();
		void				postMethod();
		void				deleteMethod();
};

