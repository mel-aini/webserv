#ifndef CONFIGFILE_HPP
#define CONFIGFILE_HPP

#include <iostream>
#include <sstream>
#include <fstream>
#include <utility>
#include <vector>

#include "../server/Server.hpp"

enum {
	SERVER,
	OPEN_BRACKET,
	CLOSE_BRACKET,
	LISTEN,
	SERVER_NAME,
	LOCATION,
	ROOT,
	INDEX,
	ERROR_PAGE,
	CLIENT_MAX_BODY_SIZE,
	ALLOW_METHODS,
	RETURN,
	AUTO_INDEX,
	CGI_EXEC,
	CGI_ALLOWED_METHODS,
	ACCEPT_UPLOAD,
	UPLOAD_LOCATION,
	WORD,
	END_OF_LINE,
	END_OF_BRACKET
};

typedef struct s_ckeckDup
{
	bool	locationHasCloseBracket;
	bool	hasRoot;
	bool	hasIndex;
	bool	hasClientMaxBodySize;
	bool	hasAllowMethods;
	bool	hasReturn;
	bool	hasAutoIndex;
	bool	hasCgiExec;
	bool	hasCgiAllowMethods;
	bool	hasAcceptUpload;
	bool	hasUploadLocation;
}			t_checkDup;

std::vector<Server>	parser(int ac, char* av[]);
std::vector<std::pair<int, std::string> >	tokenizer(char *file);
void	printError(std::string name);
int		toInt(std::string str);
std::string	toStr(int num);
int	isNum(std::string str);
int	isAlpha(std::string str);
int	isAlnum(std::string str);
int	isPath(std::string str);
int	isFile(std::string str);
int	isUrl(std::string str);
int	isExtension(std::string str);
int	isIp(std::string str);
int	isWord(std::string str);
int	isLocationPath(std::string str);

#endif
