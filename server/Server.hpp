#pragma once
#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Client.hpp"
#include "Location.hpp"
#include <poll.h>
#include "../public/Colors.hpp"
#include <fcntl.h>
#include <unistd.h>

class Server
{
	private:
		std::string						port;
		std::string						host;
		std::string						serverName;
		std::vector<Location>			locations;
		int								socket;
		std::vector<Client>				clients;
		std::vector<Server>::iterator	serversBegin;
		std::vector<Server>::iterator	serversEnd;

	public:
		Server();
		~Server();

		Server(std::string port, std::string host);
		std::string getPort() const;
		std::string getHost() const;
		int getSocket() const;
		std::vector<Client>& getClients();

		void	setPort(std::string port);
		void	setSocket(int socket);

		// oth
		void	setListen(std::pair<std::string, std::string> listen);
		void	setServerName(std::string serverName);
		void	setLocations(Location location, std::string num);
		void	setServersBegin(std::vector<Server>::iterator it);
		void	setServersEnd(std::vector<Server>::iterator it);
		std::string	eraseSlash(std::string path);

		std::vector<Location>	&getLocations(void);
		std::string	&getPort(void);
		std::string	&getHost(void);
		std::string	&getServerName(void);
		std::vector<Server>::iterator	&getServersBegin();
		std::vector<Server>::iterator	&getServersEnd();

		void	addClient(std::vector<struct pollfd> &pollfds);
		bool	processFd(std::vector<struct pollfd> &pollfds, struct pollfd *pollfd);
		bool	isClient(struct pollfd *pollfd, std::vector<Client>::iterator &it);
		void	logClients();
		void	removeClient(std::vector<struct pollfd> &pollfds, std::vector<Client>::iterator &it);
		bool	hostsMatch(std::vector<Client>::iterator& it);
		void	findRelatedHost(std::vector<Client>::iterator& it);
		void	transferClient(std::vector<Client>::iterator& it);

		class ClientFailed : public std::exception {
			public:
				const char * what() const throw();
		};
};
