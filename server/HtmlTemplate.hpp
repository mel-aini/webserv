#pragma once
#include <iostream>
#include <sstream>

class HtmlTemplate
{
	private:
		std::string	html;

	public:
		HtmlTemplate(int status, std::string& message);
		~HtmlTemplate();

		const std::string& getHtml() const;
		void	addTag(std::string tag);
};
