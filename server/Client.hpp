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

enum proccess_response {
	INITIAL,
	SENDING,
	PROCESSED,
};

class Client
{
	private:
		int					fd;
		struct sockaddr_in	address;
		struct pollfd		*pollfd;
		Request				request;
		Response			response;
		int					processing_level;
		bool				isAllowedMethod;

	public:
		Client(int fd, struct sockaddr_in address);
		~Client();

		int					getFd() const;
		struct sockaddr_in	getAddress() const;
		void				log();
		bool				readRequest(struct pollfd *pollfd);
		void				createResponse(std::vector<Location> &locations);
		void				reqHasRead();
		void				resHasSent();
		void				reset();
		void				setPollfd(struct pollfd	*pollfd);
		void				getMethod();
		void				postMethod();
		void				deleteMethod();
		bool				methodIsAllowed(std::vector<std::string> &allowMethods, std::string method);
		void				executeMethods();
};

