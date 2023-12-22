#include "ConfigFile.hpp"

using std::cout;
using std::cerr;
using std::endl;

int		toInt(std::string str)
{
	int					num;
	std::stringstream	ss;

	ss << str;
	ss >> num;
	return (num);
}

int	isNum(std::string str)
{
	size_t i = 0;
	size_t len = str.length();

	if (len == 0)
		return (0);
	while (str[i])
	{
		if (!isnumber(str[i]))
			return (0);
		i++;
	}
	return (1);
}

int	isAlpha(std::string str)
{
	size_t i = 0;
	size_t len = str.length();

	if (len == 0)
		return (0);
	while (str[i])
	{
		if (!isalpha(str[i]) && str[i] != '_')
			return (0);
		i++;
	}
	return (1);
}

int	isWhiteSpaces(std::string str)
{
	size_t i = 0;
	size_t len = str.length();

	if (len == 0)
		return (0);
	while (str[i])
	{
		if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n')
			return (0);
		i++;
	}
	return (1);
}

int	isAlnum(std::string str)
{
	size_t i = 0;
	size_t len = str.length();

	if (len == 0)
		return (0);
	while (str[i])
	{
		if (!isalnum(str[i]))
			return (0);
		i++;
	}
	return (1);
}

int	isPath(std::string str)
{
	size_t i = 0;
	size_t len = str.length();

	if (len == 0)
		return (0);
	while (str[i])
	{
		if (!isalpha(str[i]) && str[i] != '.' && str[i] != '/' && str[i] != '_')
			return (0);
		i++;
	}
	return (1);
}

int	isFile(std::string str)
{
	size_t i = 0;
	size_t len = str.length();

	if (len == 0)
		return (0);
	while (str[i])
	{
		if (!isalnum(str[i]) && str[i] != '.' && str[i] != '_')
			return (0);
		i++;
	}
	return (1);
}

int	isUrl(std::string str)
{
	size_t i = 0;
	size_t len = str.length();

	if (len == 0)
		return (0);
	while (str[i])
	{
		if (!isalpha(str[i]) && str[i] != '.' && str[i] != '/' && str[i] != ':')
			return (0);
		i++;
	}
	return (1);
}

int	isExtension(std::string str)
{
	size_t i = 0;
	size_t len = str.length();

	if (len <= 1 || str[i] != '.')
		return (0);
	i++;
	while (str[i])
	{
		if (!isalpha(str[i]))
			return (0);
		i++;
	}
	return (1);
}

int	isIp(std::string str)
{
	size_t i = 0;
	size_t len = str.length();

	if (len == 0)
		return (0);
	if (str[len - 1] == '.')
		return (0);
	while (str[i])
	{
		if ((!isnumber(str[i]) && str[i] != '.') || (!isnumber(str[i]) && str[i] == '.' && str[i + 1] && str[i + 1] == '.'))
			return (0);
		i++;
	}
	return (1);
}

size_t	skipSpaces(size_t i, std::string tmp)
{
	for (; i < tmp.length(); i++)
		if (tmp[i] != '\t' && tmp[i] != ' ')
			break ;
	return (i);
}

size_t	skipUntilSpaces(size_t i, std::string tmp)
{
	for (; i < tmp.length(); i++)
		if (tmp[i] == '\t' || tmp[i] == ' ')
			break ;
	return (i);
}

size_t	skipSpacesNewLines(size_t i, std::string tmp)
{
	for (; i < tmp.length(); i++)
		if (tmp[i] != '\t' && tmp[i] != ' ' && tmp[i] != '\n')
			break ;
	return (i);
}

int	isWord(std::string str)
{
	size_t i = 0;
	size_t len = str.length();

	if (len == 0)
		return (0);
	while (str[i])
	{
		if (!isalnum(str[i]) && str[i] != '.' && str[i] != '/' && str[i] != '_' && str[i] != ':')
			return (0);
		i++;
	}
	return (1);
}

std::pair<int, std::string>	tokenizeWords(int tokNum, std::string word)
{
	std::pair<int, std::string>	pair;

	pair.first = tokNum;
	pair.second = word;
	return (pair);
}

void	printError(std::string name)
{
	cerr << name << endl;
	exit(EXIT_FAILURE);
}

std::vector<std::pair<int, std::string> >	tokenizer(char *file)
{
	std::vector<std::pair<int, std::string> >	tokens;
	std::string	tmp;
	std::ifstream	infile(file);
	std::string	data;
	std::stringstream	ss;

	while (getline(infile, tmp, '\n'))
	{
		ss.clear();
		ss << tmp;
		while (ss >> data)
		{
			if (data == "server")
				tokens.push_back(tokenizeWords(SERVER, "server"));
			else if (data == "server_name")
				tokens.push_back(tokenizeWords(SERVER_NAME, "SERVER_NAME"));
			else if (data == "location")
				tokens.push_back(tokenizeWords(LOCATION, "LOCATION"));
			else if (data == "root")
				tokens.push_back(tokenizeWords(ROOT, "root"));
			else if (data == "{")
			{
				tokens.push_back(tokenizeWords(OPEN_BRACKET, "{"));
				tokens.push_back(tokenizeWords(END_OF_BRACKET, "eob"));
			}
			else if (data == "}")
			{
				tokens.push_back(tokenizeWords(CLOSE_BRACKET, "}"));
				tokens.push_back(tokenizeWords(END_OF_BRACKET, "eob"));
			}
			else if (data == "listen")
				tokens.push_back(tokenizeWords(LISTEN, "listen"));
			else if (data == "index")
				tokens.push_back(tokenizeWords(INDEX, "index"));
			else if (data == "error_page")
				tokens.push_back(tokenizeWords(ERROR_PAGE, "error_page"));
			else if (data == "client_max_body_size")
				tokens.push_back(tokenizeWords(CLIENT_MAX_BODY_SIZE, "client_max_body_size"));
			else if (data == "allow_methods")
				tokens.push_back(tokenizeWords(ALLOW_METHODS, "allow_methods"));
			else if (data == "return")
				tokens.push_back(tokenizeWords(RETURN, "return"));
			else if (data == "auto_index")
				tokens.push_back(tokenizeWords(AUTO_INDEX, "auto_index"));
			else if (data == "cgi_exec")
				tokens.push_back(tokenizeWords(CGI_EXEC, "cgi_exec"));
			else if (data == "cgi_allowed_methods")
				tokens.push_back(tokenizeWords(CGI_ALLOWED_METHODS, "cgi_allowed_methods"));
			else if (data == "accept_upload")
				tokens.push_back(tokenizeWords(ACCEPT_UPLOAD, "accept_upload"));
			else if (data == "upload_location")
				tokens.push_back(tokenizeWords(UPLOAD_LOCATION, "upload_location"));
			else if (isWord(data))
				tokens.push_back(tokenizeWords(WORD, data));
			else
				printError("invalid config file");
		}
		// if (tokens[tokens.size() - 1].first == WORD)
		// {
			tokens.push_back(tokenizeWords(END_OF_LINE, "eol"));
		// }
	}
	// cout << tokens.size() << endl;
	// for (std::vector<std::pair<int, std::string> >::iterator it = tokens.begin(); it != tokens.end(); it++)
	// 	cout << it->first << " >>> " << it->second << endl;
	return (tokens);
}

Location	defLocation()
{
	std::vector<std::string>	index;
	index.push_back("index.html");
	index.push_back("index.htm");
	std::vector<std::string>	allowMethods;
	allowMethods.push_back("GET");
	allowMethods.push_back("POST");
	allowMethods.push_back("DELETE");
	std::vector<std::string>	cgiExec;
	std::vector<std::string>	cgiAllowMethods;
	return (Location("/", "root", index, 50, allowMethods, "", true, cgiExec, cgiAllowMethods, true, "/upload"));
}

std::string	parseOneStrArg(std::vector<std::pair<int, std::string> >::iterator &it, bool & hasData, std::string name)
{
	std::string data;

	if (hasData && name == "server_name")
		printError("server: duplicated " + name);
	else if (hasData && (name == "root" || name == "return" || name == "upload_location"))
		printError("location: duplicated " + name);
	it += 1;
	if (name == "return")
	{
		if (it->first == WORD && (it + 1)->first == END_OF_LINE)
		{
			if (isUrl(it->second))
			{
				data = it->second;
				it += 1;
			}
			else
				printError("return: URL argument required");
		}
	}
	else if (name == "upload_location")
	{
		if (it->first == WORD && (it + 1)->first == END_OF_LINE)
		{
			if (isPath(it->second))
			{
				data = it->second;
				it += 1;
			}
			else
				printError("upload_location: path argument required");
		}
	}
	else if (it->first == WORD && (it + 1)->first == END_OF_LINE)
	{
		if (isAlnum(it->second))
		{
			data = it->second;
			it += 1;
		}
		else
			printError(name + ": alphanumeric argument required");
	}
	else
		printError(name + ": invalid argument");
	hasData = 1;
	return (data);
}

std::pair<std::string, std::string>	parseListen(std::vector<std::pair<int, std::string> >::iterator &it, std::vector<std::pair<int, std::string> > &tokens, bool & hasListen)
{
	std::pair<std::string, std::string>	listen;
	int hasPort = 0;
	int hasHost = 0;

	if (hasListen)
		printError("server: duplicated listen");
	it += 1;
	for (; it != tokens.end(); it++)
	{
		if (it->first == WORD && isNum(it->second))
		{
			if (!hasPort)
			{
				hasPort = 1;
				int num = toInt(it->second);
				if (num >= 0 && num < 65536)
					listen.first = it->second;
				else
					printError("listen: port must be in range [0 - 65535]");
			}
			else
				printError("listen: duplicated port");
		}
		else if (it->first == WORD && (isIp(it->second) || it->second == "localhost"))
		{
			if (!hasHost)
			{
				hasHost = 1;
				listen.second = it->second;
			}
			else
				printError("listen: duplicated host");
		}
		else if (it->first == END_OF_LINE)
			break ;
		else
			printError("listen: invalid argument");
	}
	hasListen = 1;
	return (listen);
}

std::vector<std::string>	parseIndex(std::vector<std::pair<int, std::string> >::iterator &it, std::vector<std::pair<int, std::string> > &tokens, bool & hasIndex)
{
	std::vector<std::string> index;

	if (hasIndex)
		printError("location: duplicated index");
	it += 1;
	for (; it != tokens.end(); it++)
	{
		if (it->first == WORD)
		{
			if (isFile(it->second))
				index.push_back(it->second);
			else
				printError("index: invalid argument");
		}
		else if (it->first == END_OF_LINE)
			break ;
		else
			printError("index: invalid argument");
	}
	hasIndex = 1;
	return (index);
}

std::pair<std::string, std::vector<int> >	parseErrorPage(std::vector<std::pair<int, std::string> >::iterator &it, std::vector<std::pair<int, std::string> > &tokens)
{
	// std::vector<std::pair<std::string, std::vector<int> > >	errorPages;
	int hasFile = 0;
	int hasCodes = 0;

	it += 1;
	std::pair<std::string, std::vector<int> >	tmpErrorPage;
	if (isPath(it->second) && !hasFile)
	{
		hasFile = 1;
		tmpErrorPage.first = it->second;
	}
	else
		printError("error_page: arguments must be \'path\' then \'error codes\'");
	it += 1;
	for (; it != tokens.end(); it++)
	{
		if (it->first == WORD)
		{
			if (isNum(it->second))
			{
				hasCodes = 1;
				int num = toInt(it->second);
				if (num >= 400 && num <= 599)
					tmpErrorPage.second.push_back(num);
				else
					printError("error_page: code must be in range [400 - 599]");
			}
			else
				printError("error_page: arguments must be \'path\' then \'error codes\'");
		}
		else if (it->first == END_OF_LINE)
			break ;
		else
			printError("error_page: arguments must be \'path\' then \'error codes\'");
	}
	if (!hasFile && !hasCodes)
		printError("error_page: arguments must be \'path\' then \'error codes\'");
	// errorPages.push_back(tmpErrorPage);
	return (tmpErrorPage);
}

int	parseClientMaxBodySize(std::vector<std::pair<int, std::string> >::iterator &it, bool &hasClientMaxBodySize)
{
	int	clientMaxBodySize = 0;

	if (hasClientMaxBodySize)
		printError("location: duplicated client_max_body_size");
	if ((it + 1)->first == WORD && (it + 2)->first == END_OF_LINE)
	{
		it += 1;
		if (it->second[it->second.length() - 1] == 'M')
		{
			it->second.erase(it->second.length() - 1);
			if (isNum(it->second))
				clientMaxBodySize = toInt(it->second);
			else
				printError("client_ma_body_size: invalid unit \'M\' or numeric argument required");
		}
		else
			printError("client_ma_body_size: invalid unit \'M\' or numeric argument required");
		it += 1;
	}
	else
		printError("client_ma_body_size: invalid argument");
	hasClientMaxBodySize = 1;
	return (clientMaxBodySize);
}

bool	parseBool(std::vector<std::pair<int, std::string> >::iterator &it, bool &hasData, std::string name)
{
	bool	data = false;

	if (hasData)
		printError("location: duplicated " + name);
	if ((it + 1)->first == WORD && (it + 2)->first == END_OF_LINE)
	{
		it += 1;
		if (it->second == "on")
			data = true;
		else if (it->second == "off")
			data = false;
		else
			printError(name + ": argument must be \'on\' or \'off\'");
		it += 1;
	}
	else
		printError(name + ": invalid argument");
	hasData = 1;
	return (data);
}

std::vector<std::string> parseAllowMethods(std::vector<std::pair<int, std::string> >::iterator &it, std::vector<std::pair<int, std::string> > &tokens, bool &hasAllowMethods)
{
	std::vector<std::string>	allowMethods;
	if (hasAllowMethods)
		printError("location: duplicated allow_methods");
	int hasGet = 0;
	int hasPost = 0;
	int hasDelete = 0;

	if ((it + 1)->first != WORD)
		printError("allow_methods: argument must be \'GET\' or \'POST\' or \'DELETE\'");
	it += 1;
	for (; it != tokens.end(); it++)
	{
		if (it->second == "GET")
		{
			if (!hasGet)
			{
				hasGet = 1;
				allowMethods.push_back(it->second);
			}
			else
				printError("allow_methods: duplicated method \'GET\'");
		}
		else if (it->second == "POST")
		{
			if (!hasPost)
			{
				hasPost = 1;
				allowMethods.push_back(it->second);
			}
			else
				printError("allow_methods: duplicated method \'POST\'");
		}
		else if (it->second == "DELETE")
		{
			if (!hasDelete)
			{
				hasDelete = 1;
				allowMethods.push_back(it->second);
			}
			else
				printError("allow_methods: duplicated method \'DELETE\'");
		}
		else if (it->first == END_OF_LINE)
			break ;
		else
			printError("allow_methods: argument must be \'GET\' or \'POST\' or \'DELETE\'");
	}
	hasAllowMethods = 1;
	return (allowMethods);
}

std::vector<std::string>	parseCgiExec(std::vector<std::pair<int, std::string> >::iterator &it, bool &hasCgiExec)
{
	std::vector<std::string>	cgiExec;

	if (hasCgiExec)
		printError("location: duplicated cgi_exec");
	if ((it + 1)->first == WORD && (it + 2)->first == WORD && (it + 3)->first == END_OF_LINE)
	{
		it += 1;
		if (isPath(it->second) && isExtension((it + 1)->second))
		{
			cgiExec.push_back(it->second);
			cgiExec.push_back((it + 1)->second);
		}
		else
			printError("cgi_exec: argument must be \'path of the program\' then \'extension\'");
		it += 2;
	}
	else
		printError("cgi_exec: argument must be \'path of the program\' then \'extension\'");
	hasCgiExec = 1;
	return (cgiExec);
}

std::vector<std::string>	parseCgiAllowMethods(std::vector<std::pair<int, std::string> >::iterator &it, std::vector<std::pair<int, std::string> > &tokens, bool &hasCgiAllowMethods)
{
	std::vector<std::string>	cgiAllowMethods;

	if (hasCgiAllowMethods)
		printError("location: duplicated cgi_allow_methods");
	int hasGet = 0;
	int hasPost = 0;

	if ((it + 1)->first != WORD)
		printError("cgi_allowed_methods: argument must be \'GET\' or \'POST\'");
	it += 1;
	for (; it != tokens.end(); it++)
	{
		if (it->second == "GET")
		{
			if (!hasGet)
			{
				hasGet = 1;
				cgiAllowMethods.push_back(it->second);
			}
			else
				printError("cgi_allowed_methods: duplicated method \'GET\'");
		}
		else if (it->second == "POST")
		{
			if (!hasPost)
			{
				hasPost = 1;
				cgiAllowMethods.push_back(it->second);
			}
			else
				printError("cgi_allowed_methods: duplicated method \'POST\'");
		}
		else if (it->first == END_OF_LINE)
			break ;
		else
			printError("cgi_allowed_methods: argument must be \'GET\' or \'POST\'");
	}
	hasCgiAllowMethods = 1;
	return (cgiAllowMethods);
}

std::pair<std::string, std::vector<int> >	defErrorPage()
{
	std::pair<std::string, std::vector<int> >	defErrorPage;
	std::vector<int>	tmpCodes;

	tmpCodes.push_back(400);
	tmpCodes.push_back(401);
	tmpCodes.push_back(402);
	tmpCodes.push_back(403);
	tmpCodes.push_back(404);
	defErrorPage.first = "error.html";
	defErrorPage.second = tmpCodes;
	return (defErrorPage);
}

Location	initializeLocation()
{
	Location	location;

	location.setPath("/");
	location.setRoot("root");
	std::vector<std::string> index;
	index.push_back("index.html");
	index.push_back("index.htm");
	location.setIndex(index);
	location.setClientMaxBodySize(50);
	std::vector<std::string> allowMethods;
	allowMethods.push_back("GET");
	allowMethods.push_back("POST");
	allowMethods.push_back("DELETE");
	location.setAllowMethods(allowMethods);
	location.setAutoIndex(true);
	location.setAcceptUpload(true);
	location.setUploadLocation("/upload");
	return (location);
}
Server	initializeServer()
{
	Server server;
	std::pair<std::string, std::string>	listen;
	listen.first = "8080";
	listen.second = "localhost";
	server.setListen(listen);
	server.setServerName("name");
	return (server);
}

std::vector<Server>	parser(char *file)
{
	std::vector<std::pair<int, std::string> > tokens = tokenizer(file);
	std::vector<Server>	servers;

	for (std::vector<std::pair<int, std::string> >::iterator it = tokens.begin(); it != tokens.end(); it++)
	{
		bool	serverHasCloseBracket = false;
		Server	tmpServer = initializeServer();

		if (it->first == SERVER)
		{
			it += 1;
			for (; it != tokens.end(); it++)
				if (it->first != END_OF_LINE)
					break ;
			if (it == tokens.end())
				printError("server: open bracket required");
			if ((it->first == OPEN_BRACKET) && ((it + 1)->first == END_OF_BRACKET))
				it += 2;
			else
				printError("server: open bracket required");
			for (; it != tokens.end(); it++)
				if (it->first != END_OF_LINE)
					break ;
			if (it == tokens.end())
				printError("server: close bracket required");
			bool	hasListen = false;
			bool	hasServerName = false;
			for (; it != tokens.end(); it++)
			{
				for (; it != tokens.end(); it++)
					if (it->first != END_OF_LINE)
						break ;
				if (it == tokens.end())
					break ;
				if (it->first == LISTEN)
					tmpServer.setListen(parseListen(it, tokens, hasListen));
				else if (it->first == SERVER_NAME)
					tmpServer.setServerName(parseOneStrArg(it, hasServerName, "server_name"));
				else if (it->first == LOCATION)
				{
					Location	tmpLocation = initializeLocation();
					if ((it + 1)->first == WORD)
					{
						it += 1;
						if (isPath(it->second))
							tmpLocation.setPath(it->second);
						else
							printError("location: invalid path");
						it += 1;
						for (; it != tokens.end(); it++)
							if (it->first != END_OF_LINE)
								break ;
						if (it == tokens.end())
							printError("location: open bracket required");
						if ((it->first == OPEN_BRACKET) && ((it + 1)->first == END_OF_BRACKET))
							it += 2;
						else
							printError("location: open bracket required");
					}
					else
						printError("location: invalid path");
					for (; it != tokens.end(); it++)
						if (it->first != END_OF_LINE)
							break ;
					if (it == tokens.end())
						printError("location: close bracket required");
					t_checkDup	check;
					std::memset(&check, 0, sizeof(t_checkDup));
					for (; it != tokens.end(); it++)
					{
						for (; it != tokens.end(); it++)
							if (it->first != END_OF_LINE)
								break ;
						if (it == tokens.end())
							break ;
						if (it->first == ROOT)
							tmpLocation.setRoot(parseOneStrArg(it, check.hasRoot, "root"));
						else if (it->first == INDEX)
							tmpLocation.setIndex(parseIndex(it, tokens, check.hasIndex));
						else if (it->first == ERROR_PAGE)
							tmpLocation.setErrorPages(parseErrorPage(it, tokens));
						else if (it->first == CLIENT_MAX_BODY_SIZE)
							tmpLocation.setClientMaxBodySize(parseClientMaxBodySize(it, check.hasClientMaxBodySize));
						else if (it->first == RETURN)
							tmpLocation.setRedirection(parseOneStrArg(it, check.hasReturn, "return"));
						else if (it->first == AUTO_INDEX)
							tmpLocation.setAutoIndex(parseBool(it, check.hasAutoIndex, "auto_index"));
						else if (it->first == ALLOW_METHODS)
							tmpLocation.setAllowMethods(parseAllowMethods(it, tokens, check.hasAllowMethods));
						else if (it->first == CGI_EXEC)
							tmpLocation.setCgiExec(parseCgiExec(it, check.hasCgiExec));
						else if (it->first == CGI_ALLOWED_METHODS)
							tmpLocation.setCgiAllowMethods(parseCgiAllowMethods(it, tokens, check.hasCgiAllowMethods));
						else if (it->first == ACCEPT_UPLOAD)
							tmpLocation.setAcceptUpload(parseBool(it, check.hasAcceptUpload, "accept_upload"));
						else if (it->first == UPLOAD_LOCATION)
							tmpLocation.setUploadLocation(parseOneStrArg(it, check.hasUploadLocation, "upload_location"));
						else if (it->first == CLOSE_BRACKET)
						{
							it += 1;
							check.locationHasCloseBracket = 1;
							break ;
						}
						else
							printError("location: invalid name \'" + it->second + "\'");
					}
					if (!check.locationHasCloseBracket)
						printError("location: close bracket required");
					if (tmpLocation.getErrorPages().size() == 0)
						tmpLocation.setErrorPages(defErrorPage());
					tmpServer.setLocations(tmpLocation);
				}
				else if (it->first == CLOSE_BRACKET)
				{
					it += 1;
					serverHasCloseBracket = 1;
					break ;
				}
				else // if (it != tokens.end() && it->first != END_OF_LINE)
				{
					cout << it->first << endl;
					printError("server: invalid names \'" + it->second + "\'");
				}
			}
			if (!serverHasCloseBracket)
				printError("server: close bracket required");
			if (tmpServer.getLocations().size() == 0)
				tmpServer.setLocations(defLocation());
			servers.push_back(tmpServer);
		}
		else if (it->first != END_OF_LINE)
			printError("config_file: invalid server context");
	}
	if (servers.size() == 0)
		printError("config_file: at least one server context required");
	return (servers);
}
