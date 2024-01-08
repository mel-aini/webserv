#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include "../public/Colors.hpp"
#include <poll.h>
#include <sstream>
#include <cstring>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include "Request.hpp"
#include "Response.hpp"
#include <ctime>
#include <map>

#define CLIENT_TIMEOUT 30

enum proccess_response {
	INITIAL,
	SENDING,
	PROCESSED,
};

class Client
{
	private:
		int									fd;
		struct sockaddr_in					address;
		struct pollfd						*pollfd;
		Request								request;
		Response							response;
		int									processing_level;
		bool								isAllowedMethod;
		time_t								logtime;
		time_t								logtime_start;
		std::map<std::string, std::string>	serverInfo;
		Location							*location;

	public:
		Client(int fd, struct sockaddr_in address);
		~Client();

        bool				findLocation(std::vector<Location> &locations, std::string uri);
		int					getFd() const;
		struct sockaddr_in	getAddress() const;
		bool				readRequest(std::vector<Location> &locations, struct pollfd *pollfd);
		bool				createResponse();
		void				reqHasRead();
		void				resHasSent();
		void				reset();
		void				setPollfd(struct pollfd	*pollfd);
		bool				getMethod(Location *location);
		void				postMethod();
		void				deleteMethod();
		bool				methodIsAllowed(std::vector<std::string> &allowMethods, std::string method);
		void				send_response();
		bool				checkLogTime();
		Request				getRequest() const;
		void				setServerInfo(std::string port, std::string host, std::string s_name);
		void				log();
		void				log_level();
};

