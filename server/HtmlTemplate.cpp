#include "HtmlTemplate.hpp"

HtmlTemplate::HtmlTemplate(int status, std::string& message)
{
	std::stringstream s;
	s << status;

	std::string data = s.str() + " " + message;

	addTag("<head>");
		addTag("<title>");
			addTag(data);
		addTag("</title>");
	addTag("</head>");
	addTag("<body>");
		addTag("<center>");
			addTag("<h1>");
				addTag(data);
			addTag("</h1>");
			addTag("<hr>");
			addTag("<center>");
				addTag("webserv");
			addTag("</center>");
		addTag("</center>");
	addTag("</body>");
}

HtmlTemplate::~HtmlTemplate()
{
}

const std::string& HtmlTemplate::getHtml() const {
	return this->html;
}

void	HtmlTemplate::addTag(std::string tag)
{
	this->html += tag;
}