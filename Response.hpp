#pragma once
#include <iostream>

class Response {
    private:
        int         status;
        std::string body;

    public:
        Response();
        ~Response();

        const std::string& getBody() const;
        int getStatus() const;
};