#pragma once

#include <map>
#include <iostream>

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

        const std::string& getUri() const;
        int getMethod() const;
        void appendToBuffer(const char *s);
        int getStatus() const;
        void setStatus(int status);
        bool nextIsBody();
        void resetBuffer();
        std::string& getBuffer();
};