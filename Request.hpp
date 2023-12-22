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

void Request::appendToBuffer(const char *s) {
    this->buffer += s;
}

int Request::getStatus() const {
    return this->status;
}

void Request::setStatus(int status) {
    this->status = status;
}

bool Request::nextIsBody() {
    if (this->buffer.length() > 3) {
        if (this->buffer.compare(this->buffer.length() - 4, 4, "\n\r\n\r") == 0)
            return true;
        else if (this->buffer.compare(this->buffer.length() - 2, 2, "\n\n") == 0)
            return true;
    }
    return false;
}

void Request::resetBuffer() {
    this->buffer = "";
}

std::string& Request::getBuffer() {
    return this->buffer;
}

Request::~Request() {}