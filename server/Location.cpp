#include "Location.hpp"

Location::Location()
{
}

Location::Location(std::string path, std::string root, std::vector<std::string> index,
int clientMaxBodySize, std::vector<std::string> allowMethods, std::string redirection, bool autoIndex,
std::vector<std::string> cgiExec, std::vector<std::string> cgiAllowMethods, bool acceptUpload,
std::string uploadLocation)
{
		this->path = path;
		this->root = root;
		this->index = index;
		this->clientMaxBodySize = clientMaxBodySize;
		this->allowMethods = allowMethods;
		this->redirection = redirection;
		this->autoIndex = autoIndex;
		this->cgiExec = cgiExec;
		this->cgiAllowMethods = cgiAllowMethods;
		this->acceptUpload = acceptUpload;
		this->uploadLocation = uploadLocation;
}

Location::~Location()
{
}

void	Location::setPath(std::string path)
{
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

void	Location::setCgiExec(std::vector<std::string> cgiExec)
{
	this->cgiExec = cgiExec;
}

void	Location::setCgiAllowMethods(std::vector<std::string> cgiAllowMethods)
{
	this->cgiAllowMethods = cgiAllowMethods;
}

void	Location::setAcceptUpload(bool acceptUpload)
{
	this->acceptUpload = acceptUpload;
}

void	Location::setUploadLocation(std::string uploadLocation)
{
	this->uploadLocation = uploadLocation;
}

void	Location::setErrorPages(std::pair<std::string, std::vector<int> > errorPage)
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
					std::cerr << "location: duplicated error codes" << std::endl;
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

std::ostream & operator<<(std::ostream & out, const Location & obj)
{
	out << " path: " << obj.path
		<< " root: " << obj.root;

	std::vector<const std::string>::iterator it1 = obj.index.begin();
	for (; it1 != obj.index.end(); it1++)
		out << " index: " << *it1;

	out << " clientMaxBodySize: " << obj.clientMaxBodySize;

	std::vector<const std::string>::iterator it3 = obj.allowMethods.begin();
	for (; it3 != obj.allowMethods.end(); it3++)
		out << "\nallowMethods:\n" << *it3 << std::endl;

	out << " redirection: " << obj.redirection << " autoIndex: " << obj.autoIndex;

	std::vector<const std::string>::iterator it4 = obj.cgiExec.begin();
	for (; it4 != obj.cgiExec.end(); it4++)
		out << "\ncgiExec:\n" << *it4 << std::endl;
	
	std::vector<const std::string>::iterator it5 = obj.cgiAllowMethods.begin();
	for (; it5 != obj.cgiAllowMethods.end(); it5++)
		out << "\ncgiallowMethods:\n" << *it5 << std::endl;
	out << " acceptUpload: " << obj.acceptUpload << " uploadLocation: " << obj.uploadLocation << std::endl;

	size_t i = 0;
	for (; i < obj.errorPages.size(); i++)
	{
		out << "\nerrorPage :\n" << "path: " << obj.errorPages[i].first << std::endl;
		size_t j = 0;
		for (; j < obj.errorPages[i].second.size(); j++)
			out << "code: " << obj.errorPages[i].second[j] << std::endl;
	}
	return (out);
}
