#include "Request.hpp"

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