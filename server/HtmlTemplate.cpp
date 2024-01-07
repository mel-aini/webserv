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
		addTag("<link rel=\"stylesheet\" href=\"error.css\">");
	addTag("</head>");
	addTag("<body>");
		addTag("<center>");
			addTag("<h1>");
				addTag(data);
			addTag("</h1>");
			// addTag("<hr>");
			// addTag("<center>");
			// 	addTag("webserv");
			// addTag("</center>");
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
		addTag("<link rel=\"stylesheet\" href=\"autoIndex.css\">");
		addTag("<link rel=\"stylesheet\" href=\"https://fonts.googleapis.com/css2?family=Material+Symbols+Outlined:opsz,wght,FILL,GRAD@24,400,0,0\"/>");
	addTag("</head>");
	addTag("<body>");
		addTag("<div class=\"container\">");
			addTag("<div class=\"title\">");
				addTag("<span class=\"material-symbols-outlined\">folder_open</span>");
				addTag("<h1>");
					addTag(data);
				addTag("</h1>");
			addTag("</div>");
			addTag("<hr>");
			addTag("<ul>");
			if (index.size() != 0)
			{
				for (; it != index.end(); it++)
				{
					addTag("<li>");
						addTag("<a href=\""); addTag(*it); addTag("\">"); addTag(*it); addTag("</a>");
					addTag("</li>");
				}
			}
			addTag("</ul>");
		addTag("</div>");
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
