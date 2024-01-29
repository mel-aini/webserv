#include "Cgi.hpp"

Cgi::Cgi(void)
{
	this->readyToSend = 0;
	this->offset = 0;
	this->cL = -1;
	this->cgiOutput = "/tmp/" + GenerateName();
	while (access(this->cgiOutput.c_str(), F_OK) == 0)
		this->cgiOutput = "/tmp/" + GenerateName();
}

int	Cgi::getReadyToSend(void)
{
	return (this->readyToSend);
}

std::string Cgi::GenerateName()
{
    struct timeval  tv;
    long long       ms;
    std::stringstream ss;

    gettimeofday(&tv, NULL);
    ms = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
    ss << ms;
    return (ss.str());
}

char**	Cgi::getCgiEnv(std::string fileToSend, std::map <std::string, std::string> firstCgiEnv)
{
	std::map<std::string, std::string>::iterator it = firstCgiEnv.begin();
	size_t	size = firstCgiEnv.size() + 2;
	char	**env = new char*[size];
	size_t i = 0;
	for (; it != firstCgiEnv.end(); it++)
	{
		env[i] = strdup(it->second.c_str());
		i++;
	}

	std::string	variable;
	variable = "SCRIPT_FILENAME=" + fileToSend;
	env[i++] = strdup(variable.c_str());
	env[i] = NULL;
	return (env);
}

// void	freeEnv(char **env)
// {
// 	size_t i = 0;

// 	while (env[i])
// 		delete (env[i++]);
// 	delete [] (env);
// }

void	alarm_handler(int signum) {
	if (signum == SIGALRM)
		std::exit(SIGALRM);
}

void	Cgi::executeCgi(int timeout, std::string fileToSend, std::string cgiPath, std::string bodyFileName, std::map <std::string, std::string> firstCgiEnv, int method_type)
{
	while (access(this->cgiOutput.c_str(), F_OK) == 0)
		this->cgiOutput = "/tmp/" + GenerateName();
	char *arg[3] = {const_cast<char *>(cgiPath.c_str()), const_cast<char *>(fileToSend.c_str()), NULL};
	this->pid1 = fork();
	if (this->pid1 == -1)
		throw 502;
	if (this->pid1 == 0)
	{
		if (signal(SIGALRM, alarm_handler) == SIG_ERR)
			throw (502);
		alarm(timeout);
		char **env = this->getCgiEnv(fileToSend, firstCgiEnv);
		int fdes = open(this->cgiOutput.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0666);
		if (fdes == -1)
			throw 502;
		if (dup2(fdes, 1) == -1 || dup2(fdes, 2) == -1)
			throw 502;
		close(fdes);
		if (method_type == POST)
		{
			int fd = open(bodyFileName.c_str(), O_RDONLY);
			if (fd == -1)
				throw (502);
			if (dup2(fd, 0) == -1)
				throw (502);
			close(fd);
		}
		execve(arg[0], arg, env);
		throw 502;
	}
	// freeEnv(env);
}

bool	Cgi::checkTimeOut(void)
{
	int st;
	int res = waitpid(this->pid1, &st, WNOHANG);
	if (res == -1)
		throw (500);
	else if (res > 0)
	{
		if (WIFSIGNALED(st) && WTERMSIG(st) == SIGALRM)
			throw (504);
		else if (WIFEXITED(st) && WEXITSTATUS(st) != 0){
			throw (502);}
		else
			this->readyToSend = 1;
		return (true);
	}
	return (false);
}

bool	Cgi::sendCgiHeader(int socket)
{
	std::stringstream ss;
	struct stat fileInf;
	stat(this->cgiOutput.c_str(), &fileInf);
	std::ifstream result(this->cgiOutput, std::ios::binary | std::ios::in);
	if (!result.is_open()) {
		throw 502;
	}
	std::string header;
	std::string tmp;
	std::string status;
	bool	hasContentLength = 0;
	bool	hasHeader = 0;
	bool	hasContentType = 0;
	while (getline(result, tmp, '\n'))
	{
		if (tmp == "\r")
		{
			hasHeader = 1;
			break ;
		}
		if (tmp.substr(0, 14) == "Content-type: ")
			hasContentType = 1;
		if (tmp.substr(0, 16) == "Content-length: ")
		{
			ss << tmp.substr(16);
			ss >> this->cL;
			hasContentLength = 1;
		}
		if (tmp.substr(0, 8) == "Status: ")
		{
			this->offset += tmp.length();
			status = tmp.substr(8);
		}
		else
			header += tmp + "\n";
	}
	if (hasHeader)
		this->offset += header.length() + 2;
	else
	{
		header = "";
		this->offset = 0;
	}
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
		ss << (fileInf.st_size - this->offset);
		std::string size;
		ss >> size;
		header += "Content-length: " + size + "\r\n";
	}
	header += "\r\n";

	result.close();
	const char *buf = header.c_str();
	if (send(socket, buf, header.length(), 0) <= 0)
		throw ResponseFailed();
	return (true);
}

bool	Cgi::sendCgiBody(int socket)
{
	char buf[1024] = {0};
	std::ifstream result(this->cgiOutput, std::ios::binary | std::ios::in);
	if (!result.is_open()) {
		throw 502;
	}

	result.seekg(this->offset, std::ios::beg);

	if (this->cL == -1)
	{
		result.read(buf, sizeof(buf));
		int bytesRead = result.gcount();
		int s = send(socket, buf, bytesRead, 0);
		if (s <= 0)
			throw ResponseFailed();
		this->offset += s;
		if (result.eof()) {
			result.close();
			this->offset = 0;
			this->cL = -1;
			this->readyToSend = 0;
			return true;
		}
		result.close();
	}
	else
	{
		if (this->cL > 1024)
		{
			this->cL -= 1024;
			result.read(buf, sizeof(buf));
			int bytesRead = result.gcount();
			int s = send(socket, buf, bytesRead, 0);
			if (s <= 0)
				throw ResponseFailed();
			this->offset += s;
			result.close();
		}
		else
		{
			this->cL -= this->cL;
			result.read(buf, this->cL);
			int bytesRead = result.gcount();
			int s = send(socket, buf, bytesRead, 0);
			if (s <= 0)
				throw ResponseFailed();
			result.close();
			this->offset = 0;
			this->cL = -1;
			this->readyToSend = 0;
			return true;
		}
	}
	return (false);
}

// title: exceptions

const char	*Cgi::ResponseFailed::what() const throw() {
	return "Error occured while sending response";
}
