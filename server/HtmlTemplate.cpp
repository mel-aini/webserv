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

HtmlTemplate::HtmlTemplate(const std::string &path, const std::vector<std::string>& index)
{
	std::vector<const std::string>::iterator	it = index.begin();
	std::string data = "Index of " + path;

	addTag("<head>");
		addTag("<title>");
			addTag(data);
		addTag("</title>");
	addTag("</head>");
	addTag("<body>");
		addTag("<h1>");
			addTag(data);
		addTag("</h1>");
		addTag("<hr>");
		addTag("<pre>");
		if (index.size() != 0)
		{
			for (; it != index.end(); it++)
			{
				addTag("<a href=\""); addTag(*it); addTag("\">"); addTag(*it); addTag("</a><br>");
			}
		}
		addTag("</pre>");
		addTag("<hr>");
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
