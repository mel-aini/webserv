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

std::vector<Server>	parser(char *file);

#endif
