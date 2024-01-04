#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

bool	hasQueryString(std::string uri)
{
	size_t	i;
	for (i = 0; i < uri.length(); i++)
		if (uri[i] == '?')
			break ;
	if (i == uri.length())
		return (false);
	return (true);
}

char**	getCgiEnv()
{
	std::string variable;
	std::string uri = "/test.php?name=smya&age=12";
	size_t size = hasQueryString(uri) ? 5 : 4;
	char	**env = new char*[size];
	variable = "REQUEST_METHOD=GET";
	std::cout << variable << std::endl;
	env[0] = strdup(variable.c_str());

	variable = "REDIRECT_STATUS=0";
	std::cout << variable << std::endl;
	env[1] = strdup(variable.c_str());

	variable = "SCRIPT_FILENAME=public/html/test.php";
	env[2] = strdup(variable.c_str());

	if (hasQueryString(uri))
	{
		std::string queryString = uri.substr(uri.find('?') + 1);
		variable = "QUERY_STRING=" + queryString;
		std::cout << variable << std::endl;
		env[3] = strdup(variable.c_str());
		env[4] = NULL;
	}
	else
	{
		env[3] = NULL;
	}
	return (env);
}

int main() {
    execve("cgi/php-cgi", NULL, getCgiEnv());
    perror("execve:");
    return 0;
}
