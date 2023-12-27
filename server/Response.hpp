#pragma once
#include <iostream>
#include <map>
#include "Location.hpp"

class Response {
    private:
        unsigned int                        status;
        std::string                         body;
        std::map<std::string, std::string>  headers;
        Location                            *location;

    public:
        Response();
        ~Response();

        const std::string&  getBody() const;
        int                 getStatus() const;
        void                setStatus(unsigned int status);

        void    redirect(std::string uri);
		void    send_4xxResponse(unsigned int status);
        void    send_respone_line();
        void    send_headers();
        void    send_body();
        Location *findLocation(std::vector<Location> &locations, std::string uri);
};