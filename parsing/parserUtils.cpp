#include "ConfigFile.hpp"

void	printError(std::string name)
{
	std::cerr << "Error" << std::endl;
	std::cerr << name << std::endl;
	std::exit(EXIT_FAILURE);
}

int		toInt(std::string str)
{
	int					num;
	std::stringstream	ss;

	ss << str;
	ss >> num;
	return (num);
}

std::string	toStr(int num)
{
	std::string			str;
	std::stringstream	ss;

	ss << num;
	ss >> str;
	return (str);
}

int	isNum(std::string str)
{
	size_t i = 0;
	size_t len = str.length();

	if (len == 0)
		return (0);
	while (str[i])
	{
		if (!isnumber(str[i]))
			return (0);
		i++;
	}
	return (1);
}

int	isAlpha(std::string str)
{
	size_t i = 0;
	size_t len = str.length();

	if (len == 0)
		return (0);
	while (str[i])
	{
		if (!isalpha(str[i]) && str[i] != '_')
			return (0);
		i++;
	}
	return (1);
}

int	isAlnum(std::string str)
{
	size_t i = 0;
	size_t len = str.length();

	if (len == 0)
		return (0);
	while (str[i])
	{
		if (!isalnum(str[i]))
			return (0);
		i++;
	}
	return (1);
}

int	isPath(std::string str)
{
	size_t i = 0;
	size_t len = str.length();

	if (len == 0)
		return (0);
	while (str[i])
	{
		if (!isalnum(str[i]) && str[i] != '/' && str[i] != '_' && str[i] != '.' && str[i] != '-')
			return (0);
		i++;
	}
	return (1);
}

int	isLocationPath(std::string str)
{
	size_t i = 0;
	size_t len = str.length();

	if (len == 0)
		return (0);
	if (str[i] != '/')
		return (0);
	i++;
	while (str[i])
	{
		if (!isalnum(str[i]) && str[i] != '/' && str[i] != '_' && str[i] != '.' && str[i] != '-')
			return (0);
		i++;
	}
	return (1);
}

int	isFile(std::string str)
{
	size_t i = 0;
	size_t len = str.length();

	if (len == 0)
		return (0);
	while (str[i])
	{
		if (!isalnum(str[i]) && str[i] != '.' && str[i] != '_' && str[i] != '-')
			return (0);
		i++;
	}
	return (1);
}

int	isUrl(std::string str)
{
	size_t i = 0;
	size_t len = str.length();

	if (len == 0)
		return (0);
	while (str[i])
	{
		// if (!isalpha(str[i]) && str[i] != '.' && str[i] != '/' && str[i] != ':')
		if (!(str[i] >= 0 && str[i] <= 127))
			return (0);
		i++;
	}
	return (1);
}

int	isExtension(std::string str)
{
	size_t i = 0;
	size_t len = str.length();

	if (len <= 1 || str[i] != '.')
		return (0);
	i++;
	while (str[i])
	{
		if (!isalpha(str[i]))
			return (0);
		i++;
	}
	return (1);
}

int	isIp(std::string str)
{
	size_t i = 0;
	size_t len = str.length();

	if (len == 0)
		return (0);
	if (str[len - 1] == '.')
		return (0);
	while (str[i])
	{
		if ((!isnumber(str[i]) && str[i] != '.') || (!isnumber(str[i]) && str[i] == '.' && str[i + 1] && str[i + 1] == '.'))
			return (0);
		i++;
	}
	return (1);
}

int	isWord(std::string str)
{
	size_t i = 0;
	size_t len = str.length();

	if (len == 0)
		return (0);
	while (str[i])
	{
		// if (!isalnum(str[i]) && str[i] != '.' && str[i] != '/' && str[i] != '_' && str[i] != ':')
		if (!(str[i] >= 0 && str[i] <= 127))
			return (0);
		i++;
	}
	return (1);
}

void	skipSlash(std::string & str)
{
	size_t	i = 0;
	for (; i < str.length(); i++)
	{
		if (str[i] == '/' && str[i + 1])
		{
			i++;
			for (; i < str.length(); i++)
			{
				if (str[i] != '/')
					break ;
				str.erase(i, 1);
				i--;
			}
		}
	}
}
