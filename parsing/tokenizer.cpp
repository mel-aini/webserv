#include "ConfigFile.hpp"

std::pair<int, std::string>	tokenizeWords(int tokNum, std::string word)
{
	std::pair<int, std::string>	pair;

	pair.first = tokNum;
	pair.second = word;
	return (pair);
}

std::vector<std::pair<int, std::string> >	tokenizer(char *file)
{
	std::vector<std::pair<int, std::string> >	tokens;
	std::string	tmp;
	std::ifstream	infile(file);
	std::string	data;
	std::stringstream	ss;

	while (getline(infile, tmp, '\n'))
	{
		ss.clear();
		ss << tmp;
		while (ss >> data)
		{
			if (data == "server")
				tokens.push_back(tokenizeWords(SERVER, "server"));
			else if (data == "server_name")
				tokens.push_back(tokenizeWords(SERVER_NAME, "SERVER_NAME"));
			else if (data == "location")
				tokens.push_back(tokenizeWords(LOCATION, "LOCATION"));
			else if (data == "root")
				tokens.push_back(tokenizeWords(ROOT, "root"));
			else if (data == "{")
			{
				tokens.push_back(tokenizeWords(OPEN_BRACKET, "{"));
				tokens.push_back(tokenizeWords(END_OF_BRACKET, "eob"));
			}
			else if (data == "}")
			{
				tokens.push_back(tokenizeWords(CLOSE_BRACKET, "}"));
				tokens.push_back(tokenizeWords(END_OF_BRACKET, "eob"));
			}
			else if (data == "listen")
				tokens.push_back(tokenizeWords(LISTEN, "listen"));
			else if (data == "index")
				tokens.push_back(tokenizeWords(INDEX, "index"));
			else if (data == "error_page")
				tokens.push_back(tokenizeWords(ERROR_PAGE, "error_page"));
			else if (data == "client_max_body_size")
				tokens.push_back(tokenizeWords(CLIENT_MAX_BODY_SIZE, "client_max_body_size"));
			else if (data == "allow_methods")
				tokens.push_back(tokenizeWords(ALLOW_METHODS, "allow_methods"));
			else if (data == "return")
				tokens.push_back(tokenizeWords(RETURN, "return"));
			else if (data == "auto_index")
				tokens.push_back(tokenizeWords(AUTO_INDEX, "auto_index"));
			else if (data == "cgi_exec")
				tokens.push_back(tokenizeWords(CGI_EXEC, "cgi_exec"));
			else if (data == "accept_upload")
				tokens.push_back(tokenizeWords(ACCEPT_UPLOAD, "accept_upload"));
			else if (data == "upload_location")
				tokens.push_back(tokenizeWords(UPLOAD_LOCATION, "upload_location"));
			else if (data == "#")
			{
				while (ss >> data);
				break ;
			}
			else if (isWord(data))
				tokens.push_back(tokenizeWords(WORD, data));
			else
				printError("invalid config file");
		}
		// if (tokens[tokens.size() - 1].first == WORD)
		// {
			tokens.push_back(tokenizeWords(END_OF_LINE, "eol"));
		// }
	}
	// cout << tokens.size() << endl;
	// for (std::vector<std::pair<int, std::string> >::iterator it = tokens.begin(); it != tokens.end(); it++)
	// 	cout << it->first << " >>> " << it->second << endl;
	return (tokens);
}
