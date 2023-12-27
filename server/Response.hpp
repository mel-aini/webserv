#pragma once
#include <iostream>
#include <map>
#include "Location.hpp"
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include "../public/Colors.hpp"

class Response {
    private:
        unsigned int                        status;
        std::string                         message;
        std::string                         body;
        std::map<std::string, std::string>  headers;
        Location                            *location;
        unsigned int                        level;

    public:
        Response();
        ~Response();

        const std::string&  getBody() const;
        int                 getStatus() const;
        void                setStatus(unsigned int status);

		void        send_4xxResponse(unsigned int status);
        void        send_status_line_and_headers(int fd);
        void        send_body(int fd);
        Location    *findLocation(std::vector<Location> &locations, std::string uri);
        void        setLocation(Location *location);
        Location    *getLocation();
        void        redirect(int fd, const std::string& location);

        class ResponseFailed : public std::exception {
			public:
				const char * what() const throw();
		};
};