#include "Cgi.hpp"

Cgi::Cgi(void)
{
	this->offset = 0;
}

char**	Cgi::getCgiEnv(std::string fileToSend, std::map <std::string, std::string> firstCgiEnv)
{
	std::map<std::string, std::string>::iterator it = firstCgiEnv.begin();
	size_t	size = firstCgiEnv.size() + 2;
	char	**env = new char*[size];
	std::cout << size << std::endl;
	size_t i = 0;
	for (; it != firstCgiEnv.end(); it++)
	{
		std::cout << it->second << "         " << i << std::endl;
		env[i] = strdup(it->second.c_str());
		i++;
	}

	std::string	variable;
	variable = "SCRIPT_FILENAME=" + fileToSend;
	std::cout << variable << "                 " << i << std::endl;
	env[i++] = strdup(variable.c_str());
	env[i] = NULL;
	return (env);
}

void	freeEnv(char **env)
{
	size_t i = 0;

	while (env[i++])
		free(env[i]);
	free(env);
}

void	Cgi::executeCgi(std::string fileToSend, std::string cgiPath, std::string bodyFileName, std::map <std::string, std::string> firstCgiEnv, int method_type)
{
	char **env = this->getCgiEnv(fileToSend, firstCgiEnv);
	pid_t	pid = fork();
	if (pid == -1)
		throw 502;
	if (pid == 0)
	{
			std::cout << bodyFileName << std::endl;
		int fdes = open("/tmp/result", O_CREAT | O_RDWR | O_TRUNC, 0666);
		if (fdes == -1)
		{
			throw 502;
		}
		dup2(fdes, 1);
		close(fdes);
		if (method_type == POST)
		{
			int fd = open(bodyFileName.c_str(), O_RDONLY);
			if (fd == -1)
			{
				throw (502);
			}
			dup2(fd, 0);
			close(fd);
		}
		execve(cgiPath.c_str(), NULL, env);
		throw 502;
	}
	wait(0);
	freeEnv(env);
}

bool	Cgi::sendCgiHeader(int socket)
{
	struct stat fileInf;
	stat("/tmp/result", &fileInf);
	std::ifstream result("/tmp/result", std::ios::binary | std::ios::in);
	if (!result.is_open()) {
		throw 502;
	}
	std::string header;
	std::string tmp;
	std::string status;
	bool	hasContentLength = 0;
	bool	hasContentType = 0;
	while (getline(result, tmp, '\n'))
	{
		if (tmp == "\r")
			break ;
		if (tmp.substr(0, 14) == "Content-type: ")
			hasContentType = 1;
		if (tmp.substr(0, 16) == "Content-length: ")
			hasContentLength = 1;
		if (tmp.substr(0, 8) == "Status: ")
		{
			this->offset += tmp.length();
			status = tmp.substr(8);
		}
		else
			header += tmp + "\n";
	}
	this->offset += header.length() + 2;
	std::cout << RED << this->offset << RESET << std::endl;
	result.seekg(this->offset, std::ios::beg);
	if (status.empty())
		status = "HTTP/1.1 200 OK \r\n";
	else
		status = "HTTP/1.1 " + status + "\r\n";
	header = status + header;

	if (!hasContentType)
		header += "Content-type: text/html\r\n";
	if (!hasContentLength)
	{
		std::stringstream ss;
		ss << (fileInf.st_size - this->offset);
		std::string size;
		ss >> size;
		header += "Content-length: " + size + "\r\n";
	}
	header += "\r\n";

	result.close();
	const char *buf = header.c_str();
	if (send(socket, buf, header.length(), 0) == -1)
		throw ResponseFailed();
	return (true);
}

bool	Cgi::sendCgiBody(int socket)
{
	char buf[1024] = {0};
	std::ifstream result("/tmp/result", std::ios::binary | std::ios::in);
	if (!result.is_open()) {
		throw 502;
	}

	result.seekg(this->offset, std::ios::beg);

	result.read(buf, sizeof(buf));
	int bytesRead = result.gcount();
	int s = send(socket, buf, bytesRead, 0);
	if (s == -1)
		throw ResponseFailed();
	this->offset += s;
	if (result.eof()) {
		result.close();
		this->offset = 0;
		return true;
	}
	result.close();
	return (false);
}

// title: exceptions

const char	*Cgi::ResponseFailed::what() const throw() {
	return "Error occured while sending response";
}
