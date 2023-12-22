#include <vector>
#include "Server.hpp"

class Interface
{
	private:
		std::vector<Server>			servers;
		std::vector<struct pollfd>	pollfds;
		nfds_t						nfds;
public:
	Interface();
	~Interface();
};

Interface::Interface()
{
}

Interface::~Interface()
{
}
