#include "ConfigFile.hpp"

using std::cout;
using std::cerr;
using std::endl;

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

std::string	parseOneStrArg(std::vector<std::pair<int, std::string> >::iterator &it, bool & hasData, std::string name, int num)
{
	std::string data;

	if (hasData && name == "server_name")
		printError("server " + toStr(num) + ": duplicated " + name);
	else if (hasData && (name == "root" || name == "return" || name == "upload_location"))
		printError("location " + toStr(num) + ": duplicated " + name);
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
				printError("location " + toStr(num) + ":" + " return: URL argument required");
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
				printError("location " + toStr(num) + ":" + " upload_location: path argument required");
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
		{
			if (name == "root")
				printError("location " + toStr(num) + ":" + name + " : alphanumeric argument required");
			else if (name == "server_name")
				printError("server " + toStr(num) + ":" + name + " : alphanumeric argument required");
		}
	}
	else
	{
		if (name == "root")
			printError("location " + toStr(num) + ":" + name + " : alphanumeric argument required");
		else if (name == "server_name")
			printError("server " + toStr(num) + ":" + name + " : alphanumeric argument required");
	}
	hasData = 1;
	return (data);
}

std::pair<std::string, std::string>	parseListen(std::vector<std::pair<int, std::string> >::iterator &it, std::vector<std::pair<int, std::string> > &tokens, bool & hasListen, int num)
{
	std::pair<std::string, std::string>	listen;
	int hasPort = 0;
	int hasHost = 0;

	if (hasListen)
		printError("server " + toStr(num) + ":" + " duplicated listen");
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
					printError("server " + toStr(num) + ":" + " listen: port must be in range [0 - 65535]");
			}
			else
				printError("server " + toStr(num) + ":" + " listen: duplicated port");
		}
		else if (it->first == WORD && (isIp(it->second) || it->second == "localhost"))
		{
			if (!hasHost)
			{
				hasHost = 1;
				listen.second = it->second;
			}
			else
				printError("server " + toStr(num) + ":" + " listen: duplicated host");
		}
		else if (it->first == END_OF_LINE)
			break ;
		else
			printError("server " + toStr(num) + ":" + " listen: invalid argument");
	}
	hasListen = 1;
	return (listen);
}

std::vector<std::string>	parseIndex(std::vector<std::pair<int, std::string> >::iterator &it, std::vector<std::pair<int, std::string> > &tokens, bool & hasIndex, int num)
{
	std::vector<std::string> index;

	if (hasIndex)
		printError("location " + toStr(num) + ":" + " duplicated index");
	it += 1;
	for (; it != tokens.end(); it++)
	{
		if (it->first == WORD)
		{
			if (isFile(it->second))
				index.push_back(it->second);
			else
				printError("location " + toStr(num) + ":" + " index: invalid argument");
		}
		else if (it->first == END_OF_LINE)
			break ;
		else
			printError("location " + toStr(num) + ":" + " index: invalid argument");
	}
	hasIndex = 1;
	return (index);
}

std::pair<std::string, std::vector<int> >	parseErrorPage(std::vector<std::pair<int, std::string> >::iterator &it, std::vector<std::pair<int, std::string> > &tokens, int num)
{
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
		printError("location " + toStr(num) + ":" + "error_page: arguments must be \'path\' then \'error codes\'");
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
					printError("location " + toStr(num) + ":" + "error_page: code must be in range [400 - 599]");
			}
			else
				printError("location " + toStr(num) + ":" + "error_page: arguments must be \'path\' then \'error codes\'");
		}
		else if (it->first == END_OF_LINE)
			break ;
		else
			printError("location " + toStr(num) + ":" + "error_page: arguments must be \'path\' then \'error codes\'");
	}
	if (!hasFile && !hasCodes)
		printError("location " + toStr(num) + ":" + "error_page: arguments must be \'path\' then \'error codes\'");
	return (tmpErrorPage);
}

int	parseClientMaxBodySize(std::vector<std::pair<int, std::string> >::iterator &it, bool &hasClientMaxBodySize, int num)
{
	int	clientMaxBodySize = 0;

	if (hasClientMaxBodySize)
		printError("location " + toStr(num) + ":" + " duplicated client_max_body_size");
	if ((it + 1)->first == WORD && (it + 2)->first == END_OF_LINE)
	{
		it += 1;
		if (it->second[it->second.length() - 1] == 'M')
		{
			it->second.erase(it->second.length() - 1);
			if (isNum(it->second))
				clientMaxBodySize = toInt(it->second);
			else
				printError("location " + toStr(num) + ":" + " client_ma_body_size: invalid unit \'M\' or numeric argument required");
		}
		else
			printError("location " + toStr(num) + ":" + " client_ma_body_size: invalid unit \'M\' or numeric argument required");
		it += 1;
	}
	else
		printError("location " + toStr(num) + ":" + " client_ma_body_size: invalid argument");
	hasClientMaxBodySize = 1;
	return (clientMaxBodySize);
}

bool	parseBool(std::vector<std::pair<int, std::string> >::iterator &it, bool &hasData, std::string name, int num)
{
	bool	data = false;

	if (hasData)
		printError("location " + toStr(num) + ":" + " duplicated " + name);
	if ((it + 1)->first == WORD && (it + 2)->first == END_OF_LINE)
	{
		it += 1;
		if (it->second == "on")
			data = true;
		else if (it->second == "off")
			data = false;
		else
			printError("location " + toStr(num) + ": " + name + ": argument must be \'on\' or \'off\'");
		it += 1;
	}
	else
		printError("location " + toStr(num) + ": " + name + ": invalid argument");
	hasData = 1;
	return (data);
}

std::vector<std::string> parseAllowMethods(std::vector<std::pair<int, std::string> >::iterator &it, std::vector<std::pair<int, std::string> > &tokens, bool &hasAllowMethods, int num)
{
	std::vector<std::string>	allowMethods;
	if (hasAllowMethods)
		printError("location " + toStr(num) + ":" + " duplicated allow_methods");
	int hasGet = 0;
	int hasPost = 0;
	int hasDelete = 0;

	if ((it + 1)->first != WORD)
		printError("location " + toStr(num) + ":" + " allow_methods: argument must be \'GET\' or \'POST\' or \'DELETE\'");
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
				printError("location " + toStr(num) + ":" + " allow_methods: duplicated method \'GET\'");
		}
		else if (it->second == "POST")
		{
			if (!hasPost)
			{
				hasPost = 1;
				allowMethods.push_back(it->second);
			}
			else
				printError("location " + toStr(num) + ":" + " allow_methods: duplicated method \'POST\'");
		}
		else if (it->second == "DELETE")
		{
			if (!hasDelete)
			{
				hasDelete = 1;
				allowMethods.push_back(it->second);
			}
			else
				printError("location " + toStr(num) + ":" + " allow_methods: duplicated method \'DELETE\'");
		}
		else if (it->first == END_OF_LINE)
			break ;
		else
			printError("location " + toStr(num) + ":" + " allow_methods: argument must be \'GET\' or \'POST\' or \'DELETE\'");
	}
	hasAllowMethods = 1;
	return (allowMethods);
}

std::vector<std::string>	parseCgiExec(std::vector<std::pair<int, std::string> >::iterator &it, bool &hasCgiExec, int num)
{
	std::vector<std::string>	cgiExec;

	if (hasCgiExec)
		printError("location " + toStr(num) + ":" + " duplicated cgi_exec");
	if ((it + 1)->first == WORD && (it + 2)->first == WORD && (it + 3)->first == END_OF_LINE)
	{
		it += 1;
		if (isPath(it->second) && isExtension((it + 1)->second))
		{
			cgiExec.push_back(it->second);
			cgiExec.push_back((it + 1)->second);
		}
		else
			printError("location " + toStr(num) + ":" + "cgi_exec: argument must be \'path of the program\' then \'extension\'");
		it += 2;
	}
	else
		printError("location " + toStr(num) + ":" + "cgi_exec: argument must be \'path of the program\' then \'extension\'");
	hasCgiExec = 1;
	return (cgiExec);
}

std::vector<std::string>	parseCgiAllowMethods(std::vector<std::pair<int, std::string> >::iterator &it, std::vector<std::pair<int, std::string> > &tokens, bool &hasCgiAllowMethods, int num)
{
	std::vector<std::string>	cgiAllowMethods;

	if (hasCgiAllowMethods)
		printError("location " + toStr(num) + ":" + " duplicated cgi_allow_methods");
	int hasGet = 0;
	int hasPost = 0;

	if ((it + 1)->first != WORD)
		printError("location " + toStr(num) + ":" + " cgi_allowed_methods: argument must be \'GET\' or \'POST\'");
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
				printError("location " + toStr(num) + ":" + " cgi_allowed_methods: duplicated method \'GET\'");
		}
		else if (it->second == "POST")
		{
			if (!hasPost)
			{
				hasPost = 1;
				cgiAllowMethods.push_back(it->second);
			}
			else
				printError("location " + toStr(num) + ":" + " cgi_allowed_methods: duplicated method \'POST\'");
		}
		else if (it->first == END_OF_LINE)
			break ;
		else
			printError("location " + toStr(num) + ":" + " cgi_allowed_methods: argument must be \'GET\' or \'POST\'");
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

void	addServers(std::vector<Server> & servers, Server & tmpServer)
{
	std::vector<Server>::iterator	it = servers.begin();

	for (; it != servers.end(); it++)
	{
		if (tmpServer.getHost() == it->getHost() && tmpServer.getPort() == it->getPort()
			&& tmpServer.getServerName() == it->getServerName())
		{
			std::cerr << "config file: duplicated servers" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	servers.push_back(tmpServer);
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
	int serverNum = 0;

	for (std::vector<std::pair<int, std::string> >::iterator it = tokens.begin(); it != tokens.end(); it++)
	{
		bool	serverHasCloseBracket = false;
		Server	tmpServer = initializeServer();

		if (it->first == SERVER)
		{
			int hasRootLocation = false;
			int locationNum = 0;
			serverNum++;
			it += 1;
			for (; it != tokens.end(); it++)
				if (it->first != END_OF_LINE)
					break ;
			if (it == tokens.end())
				printError("server " + toStr(serverNum) + ": open bracket required");
			if ((it->first == OPEN_BRACKET) && ((it + 1)->first == END_OF_BRACKET))
				it += 2;
			else
				printError("server " + toStr(serverNum) + ": open bracket required");
			for (; it != tokens.end(); it++)
				if (it->first != END_OF_LINE)
					break ;
			if (it == tokens.end())
				printError("server " + toStr(serverNum) + ": close bracket required");
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
					tmpServer.setListen(parseListen(it, tokens, hasListen, serverNum));
				else if (it->first == SERVER_NAME)
					tmpServer.setServerName(parseOneStrArg(it, hasServerName, "server_name", serverNum));
				else if (it->first == LOCATION)
				{
					Location	tmpLocation = initializeLocation();
					locationNum++;
					if ((it + 1)->first == WORD)
					{
						it += 1;
						if (isLocationPath(it->second))
						{
							if (it->second == "/")
								hasRootLocation = true;
							tmpLocation.setPath(it->second);
						}
						else
							printError("location " + toStr(locationNum) + " : invalid path");
						it += 1;
						for (; it != tokens.end(); it++)
							if (it->first != END_OF_LINE)
								break ;
						if (it == tokens.end())
							printError("location " + toStr(locationNum) + " : open bracket required");
						if ((it->first == OPEN_BRACKET) && ((it + 1)->first == END_OF_BRACKET))
							it += 2;
						else
							printError("location " + toStr(locationNum) + " : open bracket required");
					}
					else
						printError("location " + toStr(locationNum) + " : invalid path");
					for (; it != tokens.end(); it++)
						if (it->first != END_OF_LINE)
							break ;
					if (it == tokens.end())
						printError("location " + toStr(locationNum) + " : close bracket required");
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
							tmpLocation.setRoot(parseOneStrArg(it, check.hasRoot, "root", locationNum));
						else if (it->first == INDEX)
							tmpLocation.setIndex(parseIndex(it, tokens, check.hasIndex, locationNum));
						else if (it->first == ERROR_PAGE)
							tmpLocation.setErrorPages(parseErrorPage(it, tokens, locationNum), toStr(locationNum));
						else if (it->first == CLIENT_MAX_BODY_SIZE)
							tmpLocation.setClientMaxBodySize(parseClientMaxBodySize(it, check.hasClientMaxBodySize, locationNum));
						else if (it->first == RETURN)
							tmpLocation.setRedirection(parseOneStrArg(it, check.hasReturn, "return", locationNum));
						else if (it->first == AUTO_INDEX)
							tmpLocation.setAutoIndex(parseBool(it, check.hasAutoIndex, "auto_index", locationNum));
						else if (it->first == ALLOW_METHODS)
							tmpLocation.setAllowMethods(parseAllowMethods(it, tokens, check.hasAllowMethods, locationNum));
						else if (it->first == CGI_EXEC)
							tmpLocation.setCgiExec(parseCgiExec(it, check.hasCgiExec, locationNum));
						else if (it->first == CGI_ALLOWED_METHODS)
							tmpLocation.setCgiAllowMethods(parseCgiAllowMethods(it, tokens, check.hasCgiAllowMethods, locationNum));
						else if (it->first == ACCEPT_UPLOAD)
							tmpLocation.setAcceptUpload(parseBool(it, check.hasAcceptUpload, "accept_upload", locationNum));
						else if (it->first == UPLOAD_LOCATION)
							tmpLocation.setUploadLocation(parseOneStrArg(it, check.hasUploadLocation, "upload_location", locationNum));
						else if (it->first == CLOSE_BRACKET)
						{
							it += 1;
							check.locationHasCloseBracket = 1;
							break ;
						}
						else
							printError("location " + toStr(locationNum) + " : invalid name \'" + it->second + "\'");
					}
					if (!check.locationHasCloseBracket)
						printError("location " + toStr(locationNum) + " : close bracket required");
					if (tmpLocation.getErrorPages().size() == 0)
						tmpLocation.setErrorPages(defErrorPage(), toStr(locationNum));
					tmpServer.setLocations(tmpLocation, toStr(serverNum));
				}
				else if (it->first == CLOSE_BRACKET)
				{
					it += 1;
					serverHasCloseBracket = 1;
					break ;
				}
				else
				{
					cout << it->first << endl;
					printError("server " + toStr(serverNum) + " : invalid names \'" + it->second + "\'");
				}
			}
			if (!serverHasCloseBracket)
				printError("server " + toStr(serverNum) + " : close bracket required");
			if (tmpServer.getLocations().size() == 0)
				tmpServer.setLocations(defLocation(), toStr(serverNum));
			else if (!hasRootLocation)
				tmpServer.setLocations(defLocation(), toStr(serverNum));
			addServers(servers, tmpServer);
		}
		else if (it->first != END_OF_LINE)
			printError("config_file: invalid server context");
	}
	if (servers.size() == 0)
		printError("config_file: at least one server context required");

	std::vector<Server>::iterator	serverIt = servers.begin();
	for (; serverIt != servers.end(); serverIt++)
		serverIt->setIt(servers.begin());
	return (servers);
}
