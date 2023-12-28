#pragma once

#include <map>
#include <iostream>
#include "Location.hpp"

class Request {
    private:
        int                                 method;
        std::string                         uri;
        std::map<std::string, std::string>  headers;
        std::string                         buffer;
        int             					status;

        /*
            title: hedears we will need
            1- Host
            2- Connection
            3- Content-Length
            4- Content-Type
            5- Transfer-Encoding
        */
        std::string                         body;

    public:
        Request() : status(0) {};
        ~Request();

        const std::string&  getUri() const;
        void                setUri(std::string& uri);
        int                 getMethod() const;
        void                appendToBuffer(const char *s);
        int                 getStatus() const;
        void                setStatus(int status);
        bool                nextIsBody();
        void                resetBuffer();
        int                 getSize() const;
        std::string&        getBuffer();

        class RequestFailed : public std::exception {
			public:
				const char * what() const throw();
		};
};
