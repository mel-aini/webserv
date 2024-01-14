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

enum METHOD {
	GET,
	POST,
};

class Cgi
{
	private:
		size_t	offset;
		long long	cL;

	public:
		Cgi(void);

		char**	getCgiEnv(std::string fileToSend, std::map <std::string, std::string> firstCgiEnv);
		void	executeCgi(std::string fileToSend, std::string cgiPath, std::string bodyFileName, std::map <std::string, std::string> firstCgiEnv, int method_type);
		bool	sendCgiHeader(int socket);
		bool	sendCgiBody(int socket);
		class ResponseFailed : public std::exception {
			public:
				const char * what() const throw();
		};
};


#endif
