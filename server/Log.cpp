#include "Log.hpp"

Log::Log(/* args */)
{
}

Log::~Log()
{
}

std::string&	Log::getId() {
	return this->id;
}

void			Log::setId(std::string id) {
	this->id = id;
}

void	Log::addLog(std::string type, std::string content) {
	std::ofstream out("traces/" + this->id, std::ios::app | std::ios::out);
	out << "[" + type + "] " + content << std::endl;
	out.close();
}