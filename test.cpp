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
	std::string uri = "/test.php?name=smya&age=12";
	size_t size = hasQueryString(uri) ? 4 : 3;
	char	**env = new char*[size];
	// std::memset(env, 0, 12);
	std::string	variable;

	// variable = "SERVER_NAME=name";
	// std::cout << variable << std::endl;
	// env[0] = strdup(variable.c_str());

	// variable = "SERVER_PORT=8080";
	// std::cout << variable << std::endl;
	// env[1] = strdup(variable.c_str());

	// variable = "SERVER_PROTOCOL=HTTP/1.1";
	// std::cout << variable << std::endl;
	// env[2] = strdup(variable.c_str());

	// if (uri[0] == '/')
	// 	uri.erase(0, 1);
	// variable = "SCRIPT_NAME=/" + uri.substr(0, uri.find(".php") + 4);
	// std::cout << variable << std::endl;
	// env[3] = strdup(variable.c_str());
	
	// variable = "PATH_INFO=" + uri.substr(uri.find(".php") + 4);
	// std::cout << variable << std::endl;
	// env[4] = strdup(variable.c_str());

	// variable = "REQUEST_URI=" + uri;
	// std::cout << variable << std::endl;
	// env[5] = strdup(variable.c_str());

	// variable = "STATUS=200";
	// std::cout << variable << std::endl;
	// env[6] = strdup(variable.c_str());

	variable = "REQUEST_METHOD=GET";
	std::cout << variable << std::endl;
	env[0] = strdup(variable.c_str());

	variable = "REDIRECT_STATUS=0";
	std::cout << variable << std::endl;
	env[1] = strdup(variable.c_str());

	if (hasQueryString(uri))
	{
		std::string queryString = uri.substr(uri.find('?') + 1);
		variable = "QUERY_STRING=" + queryString;
		std::cout << variable << std::endl;
		env[2] = strdup(variable.c_str());
		env[3] = NULL;
	}
	else
	{
		env[2] = NULL;
	}
	return (env);
}

int main() {
	std::string ar1 = "cgi/php-cgi";
	std::string ar2 = "public/html/test.php";
	char* arg[3] = {const_cast<char *>(ar1.c_str()), const_cast<char *>(ar2.c_str()), NULL};   

    FILE* queryFile = tmpfile();
    fprintf(queryFile, "QUERY_STRING=%s", "name=smya&age=12");
    rewind(queryFile);

    // Redirect stdin to the temporary file
    dup2(fileno(queryFile), STDIN_FILENO);

    // Execute the PHP script
    int r = execve(arg[0], arg, getCgiEnv());

    // Clean up and close the temporary file
    fclose(queryFile);

    // Free the environment array

    perror("execve:");
    printf("%d\n", r);
    return 0;
}
