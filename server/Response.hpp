#pragma once
#include <iostream>
#include <map>
#include "Location.hpp"
#include <sstream>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include "../public/Colors.hpp"
#include "HtmlTemplate.hpp"

enum sending_level{
    SENDING_HEADERS,
    SENDING_BODY,
    SENDING_END,
};

class Response {
    private:
        unsigned int                        status;
        std::string                         message;
        std::string                         body;
        std::map<std::string, std::string>  headers;
        Location                            *location;
        unsigned int                        sending_level;
        size_t                              bodyOffset;
        std::map<int, std::string>          status_codes;
        int                                 socket;

    public:
        Response();
        ~Response();

        const std::string&  getBody() const;
        int                 getStatus() const;
        void                setStatus(unsigned int status);
        std::string         getStatusMessage();

        void                setSocket(int fd);

		bool        send_response_error();
        void        send_status_line_and_headers();
        void        send_body();
        Location    *findLocation(std::vector<Location> &locations, std::string uri);
        void        setLocation(Location *location);
        Location    *getLocation();
        bool        isInErrorPages(std::string& errPage);
        void        redirect(const std::string& location);
        bool        methodNotAllowed(std::vector<std::string> &allowMethods);
        void        reset();
    
        class ResponseFailed : public std::exception {
			public:
				const char * what() const throw();
		};
};