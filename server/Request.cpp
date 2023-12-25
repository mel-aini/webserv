/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hel-mamo <hel-mamo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/23 11:54:49 by hel-mamo          #+#    #+#             */
/*   Updated: 2023/12/24 18:03:49 by hel-mamo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request()
{
    this->_state = START;
    this->_chunkState = CHUNK_SIZE_START;
    this->_lengthState = 0;
    this->_filename = "body";
}

Request::Request(Request const &src)
{
    *this = src;
}

Request &Request::operator=(Request const &rhs)
{
    if (this != &rhs)
    {
        this->_state = rhs._state;
        this->_chunkState = rhs._chunkState;
        this->_lengthState = rhs._lengthState;
        this->_request = rhs._request;
        this->_method = rhs._method;
        this->_url = rhs._url;
        this->_version = rhs._version;
        this->_headers = rhs._headers;
        this->_filename = rhs._filename;
    }
    return *this;
}



bool Request::ContentLengthExists()
{
    std::map<std::string, std::string>::iterator it = this->_headers.find("content-length");
    if (it == this->_headers.end())
        return false;
    return true;
}

bool Request::TransferEncodingExists()
{
    std::map<std::string, std::string>::iterator it = this->_headers.find("transfer-encoding");
    if (it == this->_headers.end())
        return false;
    return true;
}

bool Request::isChunked()
{
    if (this->TransferEncodingExists() && this->getTransferEncoding() == "chunked")
        return true;
    return false;
}

size_t Request::getContentLenght()
{
    std::map<std::string, std::string>::iterator it = this->_headers.find("content-length");
    if (it == this->_headers.end())
        return 0;
    return std::stoi(it->second);
}

std::string Request::getTransferEncoding()
{
    std::map<std::string, std::string>::iterator it = this->_headers.find("transfer-encoding");
    if (it == this->_headers.end())
        return "";
    return it->second;
}

int Request::readByChunk()
{
    /*
    */
    // if (this->_request.find("\r\n") == std::string::npos)
    //     return 1;
    // std::string chunkSize = this->_request.substr(0, this->_request.find("\r\n"));
    // if (chunkSize == "0")
    // {
    //     this->_state = END;
    //     return 1;
    // }
    // std::stringstream ss(chunkSize);
    // size_t sizeToread;
    // ss >> std::hex >> sizeToread;
    // // if (sizeToread == 0)
    // // {
    // //     this->_request = this->_request.substr(this->_request.find("\r\n") + 2);
    // //     this->_state = END;
    // //     return 1;
    // // }
    // // read by chunk size
    // this->_request = this->_request.substr(this->_request.find("\r\n") + 2);
    // this->_lengthState += this->_request.length();
    // if (this->_lengthState <= sizeToread)
    // {
    //     this->_body << this->_request;
    //     this->_request = "";
    //     return 1;
    // }
    // else
    // {
    //     this->_body << this->_request.substr(0, sizeToread);
    //     this->_request = this->_request.substr(sizeToread + 2);
    // }
    return ((1));
}


int Request::readRequest(char *buffer, int size)
{   
    this->_request += std::string(buffer, size);
    switch (this->_state)
    {
        case START : 
        {
            if (this->_request.find("\r\n") == std::string::npos)
                break;
            this->_state = METHOD;
        }
        case METHOD:
        {
            std::string requestLine = this->_request.substr(0, this->_request.find("\r\n"));
            std::stringstream ss(requestLine);
            std::getline(ss, this->_method, ' ');
            std::getline(ss, this->_url, ' ');
            std::getline(ss, this->_version, ' ');
            this->_request = this->_request.substr(this->_request.find("\r\n") + 2);
            this->_state = HEADER;
        }
        case HEADER :
        {
            if (this->_request.find("\r\n\r\n") == std::string::npos)
                break;
            std::string headers = this->_request.substr(0, this->_request.find("\r\n\r\n"));
            std::stringstream ss(headers);
            std::string line;
            while (std::getline(ss, line, '\n'))
            {
                if (line == "\r")
                    break;
                std::string key;
                std::string value;
                std::stringstream ss2(line);
                std::getline(ss2, key, ':');
                if (ss2.peek() == ' ')// skip the space after the :
                    ss2.seekg(1, ss2.cur);
                std::getline(ss2, value, '\r');
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                this->_headers[key] = value;
            }
            this->_request = this->_request.substr(this->_request.find("\r\n\r\n") + 4);
            if (this->ContentLengthExists())
            {
                this->_state = CONTENT_LENGTH;
                this->_lengthState = 0;
            }
            else if (this->isChunked())
            {
                this->_state = CHUNKED;
                this->_lengthState = 0;
            }
            else
                this->_state = END;
        }
        case CONTENT_LENGTH :
        {           
            /*
                if Content-Length is found in the headers
            */
            std::ofstream file(this->_filename, std::ios::out | std::ios::app);
            this->_lengthState += this->_request.length();
            if (this->_lengthState < this->getContentLenght())
            {
                file << this->_request;
                this->_request = "";
                file.close();
                break;
            }
            else
            {
                file << this->_request.substr(0, this->getContentLenght());
                this->_request = this->_request.substr(this->getContentLenght());
                file.close();
            }
            this->_state = END;  
        }
        case CHUNKED :
        {
            /*
                if Transfer-Encoding: chunked is found in the headers
            */
            // if (readByChunk())
            //     break;
           
        }
        case BOUNDARY :
        {
            /*
                if Transfer-Encoding: chunked is found in the headers
            */
        }
        case END :
        {
            return 1;
        }
    }
    return 0;
}

void Request::printRequest()
{
    std::cout << "Method: " << this->_method << std::endl;
    std::cout << "Path: " << this->_url << std::endl;
    std::cout << "Version: " << this->_version << std::endl;
    std::cout << "Headers: " << std::endl;
    for (std::map<std::string, std::string>::iterator it = this->_headers.begin(); it != this->_headers.end(); it++)
    {
        std::cout << it->first << ": " << it->second << std::endl;
    }
}
