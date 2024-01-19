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
#include "Log.hpp"
#include <chrono>

#define CLIENT_TIMEOUT 5
#define MEGABYTE 1048576

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
		Log									trace;
		time_t								logtime;
		time_t								logtime_start;
		std::map<std::string, std::string>	serverInfo;
		std::map<std::string, std::string>	firstCgiEnv;
		Location							*location;

	public:
		Client(int fd, struct sockaddr_in address);
		~Client();


		// title: getters
		struct pollfd		*getPollfd() const;
		Log&				getLog();
		int					getFd() const;
		struct sockaddr_in	getAddress() const;
		Request				getRequest() const;
		Response			getResponse() const;
		int					getProcessing_level() const;
		Location			*getLocation() const;

		// title: setters
		void				setPollfd(struct pollfd	*pollfd);
		void				setServerInfo(std::string port, std::string host, std::string s_name);
		void				setFirstCgiEnv(void);
		void				setCookies(void);

        bool				findLocation(std::vector<Location> &locations, std::string uri);
		bool				readRequest(std::vector<Location> &locations);
		bool				createResponse();
		void				reqHasRead();
		void				resHasSent();
		void				reset();
		bool				methodIsAllowed(std::vector<std::string> &allowMethods, std::string method);
		void				send_response();
		bool				checkLogTime();
		bool        		isBeyondMaxBodySize();
		void				log();
		void				log_level();
		void				log_members();

		class RequestFailed : public std::exception {
			public:
				const char * what() const throw();
		};
};

