#include "Location.hpp"

Location::Location()
{
}

Location::Location(std::string path, std::string root, std::vector<std::string> index,
int clientMaxBodySize, std::vector<std::string> allowMethods, std::string redirection, bool autoIndex,
std::pair<std::string, std::string> cgiExec, bool acceptUpload,
std::string uploadLocation)
{
		this->path = path;
		this->root = root;
		this->index = index;
		this->clientMaxBodySize = clientMaxBodySize;
		this->allowMethods = allowMethods;
		this->redirection = redirection;
		this->autoIndex = autoIndex;
		this->cgiExec.push_back(cgiExec);
		this->acceptUpload = acceptUpload;
		this->uploadLocation = uploadLocation;
}

Location::~Location()
{
}

const std::string& Location::getRedirection() const {
	return this->redirection;
}

void	Location::setPath(std::string path)
{
	if (path == "/")
	{
		this->path = path;
		return ;
	}
	if (path[path.length() - 1] == '/')
		path.erase(path.length() - 1);
	this->path = path;
}

void	Location::setRoot(std::string root)
{
	this->root = root;
}

void	Location::setIndex(std::vector<std::string> index)
{
	this->index = index;
}

void	Location::setClientMaxBodySize(int clientMaxBodySize)
{
	this->clientMaxBodySize = clientMaxBodySize;
}

void	Location::setAllowMethods(std::vector<std::string> allowMethods)
{
	this->allowMethods = allowMethods;
}

void	Location::setRedirection(std::string redirection)
{
	this->redirection = redirection;
}

void	Location::setAutoIndex(bool autoIndex)
{
	this->autoIndex = autoIndex;
}

void	Location::setCgiExec(std::pair<std::string, std::string> cgiExec, std::string num)
{
	std::vector<std::pair<std::string, std::string> >::iterator	it = this->cgiExec.begin();
	for (; it != this->cgiExec.end(); it++)
	{
		if (it->first == cgiExec.first)
		{
			std::cerr << "location " + num + ":" + " duplicated cgi program name" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	this->cgiExec.push_back(cgiExec);
}

void	Location::setAcceptUpload(bool acceptUpload)
{
	this->acceptUpload = acceptUpload;
}

void	Location::setUploadLocation(std::string uploadLocation)
{
	this->uploadLocation = uploadLocation;
}

std::vector<std::pair<std::string, std::string> >	Location::getCgiExec(void)
{
	return (this->cgiExec);
}

void	Location::setErrorPages(std::pair<std::string, std::vector<int> > errorPage, std::string num)
{
	std::vector<std::pair<std::string, std::vector<int> > >::iterator	it = this->errorPages.begin();

	for (; it != errorPages.end(); it++)
	{
		std::vector<int>::iterator	intIt = it->second.begin();
		for (; intIt != it->second.end(); intIt++)
		{
			std::vector<int>::iterator	intIt2 = errorPage.second.begin();
			for (; intIt2 != errorPage.second.end(); intIt2++)
			{
				if (*intIt == *intIt2)
				{
					std::cerr << "location " + num + ":" + " duplicated error codes" << std::endl;
					exit(EXIT_FAILURE);
				}
			}
		}
	}
	this->errorPages.push_back(errorPage);
}

std::vector<std::pair<std::string, std::vector<int> > >	Location::getErrorPages(void)
{
	return (this->errorPages);
}

std::string	Location::getPath(void)
{
	return (this->path);
}

std::string	Location::getRoot(void)
{
	return (this->root);
}

std::string	Location::getUploadLocation(void)
{
	return (this->uploadLocation);
}

std::vector<std::string>	Location::getIndex(void)
{
	return (this->index);
}

bool	Location::getAutoIndex(void)
{
	return (this->autoIndex);
}

bool	Location::getAcceptUpload(void)
{
	return (this->acceptUpload);
}

std::vector<std::string>&	Location::getAllowMethods(void)
{
	return (this->allowMethods);
}

size_t	Location::getClientMaxBodySize(void)
{
	return (this->clientMaxBodySize);
}
