#pragma once
#include <iostream>

class Request {
    private:
        int         method;
        std::string url;
        // std::string headers[];

    public:
        Request();
        ~Request();

        const std::string& getUrl() const;
        int getMethod() const;
};