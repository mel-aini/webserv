#pragma once
#include <iostream>
#include <fstream>

class Log
{
	private:
		std::string	id;

	public:
		Log();
		~Log();

		void			addLog(std::string type, std::string content);
		std::string&	getId();
		void			setId(std::string id);
};
