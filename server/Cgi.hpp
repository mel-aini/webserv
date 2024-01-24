#ifndef CGI_HPP
#define CGI_HPP

#include <iostream>
#include <map>
#include <sstream>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include "../public/Colors.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>

enum METHOD {
	GET,
	POST,
};

class Cgi
{
	private:
		std::string	cgiOutput;
		size_t		offset;
		long long	cL;
		int			readyToSend;
		pid_t		pid1;

	public:
		Cgi(void);

		char**		getCgiEnv(std::string fileToSend, std::map <std::string, std::string> firstCgiEnv);
		void		executeCgi(int timeout, std::string fileToSend, std::string cgiPath, std::string bodyFileName, std::map <std::string, std::string> firstCgiEnv, int method_type);
		bool		sendCgiHeader(int socket);
		bool		sendCgiBody(int socket);
		std::string	GenerateName();
		int			getReadyToSend(void);
		bool		checkTimeOut(void);
		class ResponseFailed : public std::exception {
			public:
				const char * what() const throw();
		};
};


#endif
