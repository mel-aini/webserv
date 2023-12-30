#pragma once
#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>

class HtmlTemplate
{
	private:
		std::string	html;

	public:
		HtmlTemplate(const std::string &path, const std::vector<std::string>& index);
		HtmlTemplate(int status, std::string& message);
		~HtmlTemplate();

		const std::string& getHtml() const;
		void	addTag(std::string tag);
};
