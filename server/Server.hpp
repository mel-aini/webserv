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
		std::string					port;
		std::string					host;
		std::string					serverName;
		std::vector<Location>		locations;
		int							socket;
		std::vector<Client>			clients;

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
		std::vector<Location>	getLocations(void);
		std::string	getPort(void);
		std::string	getHost(void);
		std::string	getServerName(void);

		void	addClient();
		bool	processFd(std::vector<struct pollfd> &pollfds, struct pollfd *pollfd, int event);
		bool	isClient(struct pollfd *pollfd, std::vector<Client>::iterator &it);
		void	logClients();
		void	removeClient(std::vector<struct pollfd> &pollfds, std::vector<Client>::iterator &it);
};
