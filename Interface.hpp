#include <vector>
#include "server.hpp"

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
