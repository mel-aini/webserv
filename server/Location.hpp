#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <iostream>
#include <vector>

class Location
{
	// private:
	public:
		std::string	path;
		std::string root;
		std::vector<std::string> index;
		int	clientMaxBodySize;
		std::vector<std::string> allowMethods;
		std::string		redirection;
		bool	autoIndex;
		std::vector<std::string> cgiExec;
		std::vector<std::string> cgiAllowMethods;
		bool	acceptUpload;
		std::string		uploadLocation;
		std::vector<std::pair<std::string, std::vector<int> > >	errorPages;

		Location(void);
		Location(std::string path, std::string root, std::vector<std::string> index, 
			int clientMaxBodySize, std::vector<std::string> allowMethods, std::string redirection, bool autoIndex,
				std::vector<std::string> cgiExec, std::vector<std::string> cgiAllowMethods, bool acceptUpload,
					std::string uploadLocation);
		~Location(void);

		void	setPath(std::string path);
		void	setRoot(std::string root);
		void	setIndex(std::vector<std::string> index);
		void	setClientMaxBodySize(int clientMaxBodySize); 
		void	setAllowMethods(std::vector<std::string> allowMethods);
		void	setRedirection(std::string redirection);
		void	setAutoIndex(bool autoIndex);
		void	setCgiExec(std::vector<std::string> cgiExec);
		void	setCgiAllowMethods(std::vector<std::string> cgiAllowMethods);
		void	setAcceptUpload(bool acceptUpload);
		void	setUploadLocation(std::string uploadLocation);
		void	setErrorPages(std::pair<std::string, std::vector<int> > errorPage);

		std::vector<std::pair<std::string, std::vector<int> > >	getErrorPages(void);
		std::string	getPath(void);
};

std::ostream & operator<<(std::ostream  & out, const Location & obj);

#endif
