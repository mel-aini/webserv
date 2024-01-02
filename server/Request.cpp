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
#include <unistd.h>

Request::Request() : status(200), _bodySize(0), _state(START), _chunkState(CHUNK_SIZE_START), _lengthState(0) , _filename("/tmp/" + std::to_string(time(0)))
{
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
        this->_uri = rhs._uri;
        this->_version = rhs._version;
        this->_headers = rhs._headers;
        this->_filename = rhs._filename;
        this->status = rhs.status;
    }
    return *this;
}

int         Request::getStatus()
{
    return this->status;
}

std::string Request::getMethod()
{
    return this->_method;
}

std::string Request::getUri()
{
    return this->_uri;
}

std::string Request::getVersion()
{
    return this->_version;
}

std::string Request::getHeader(std::string key)
{
    std::map<std::string, std::string>::iterator it = this->_headers.find(key);
    if (it == this->_headers.end())
        return "";
    return it->second;
}

std::map<std::string, std::string> Request::getHeaders()
{
    return this->_headers;
}

size_t Request::getBodysize()
{
    return this->_bodySize;
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

std::string Request::getFilename()
{
    return this->_filename;
}

std::string Request::getContentType()
{
    std::map<std::string, std::string>::iterator it = this->_headers.find("content-type");
    if (it == this->_headers.end())
        return "none";
    return it->second.substr(0, it->second.find(";"));
}

int Request::thereIsBoundary()
{
    std::map<std::string, std::string>::iterator it = this->_headers.find("content-type");
    if (it == this->_headers.end())
        return 0;
    std::string contentType = it->second;
    if (contentType.find("multipart/form-data") == std::string::npos)
        return 0;
    std::stringstream ss(contentType);
    std::string token;
    while (std::getline(ss, token, ';'))
    {
        if (token.find("boundary") != std::string::npos)
        {
            this->_boundary = token.substr(token.find("=") + 1);
            return 1;
        }
    }
    return 0;
}

int Request::validateRequestLine()
{
    std::string allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~:/?#[]@!$&'()*+,;=";
    if (this->_method != "GET" && this->_method != "POST" && this->_method != "DELETE" 
        && this->_method != "PUT" && this->_method != "HEAD" && this->_method != "OPTIONS" 
        && this->_method != "TRACE" && this->_method != "CONNECT")
    {
        this->status = 400;
        return 0;
    }
    if (this->_uri.find_first_not_of(allowed) != std::string::npos)
    {
        this->status = 400;
        return 0;
    }
    if (this->_uri[0] != '/' || this->_version != "HTTP/1.1")
    {
        this->status = 400;
        return 0;
    }
    if (this->_uri.length() > 2048)
    {
        this->status = 414;
        return 0;
    }
    return 1;
}

// int Request::validateHeaderLine()
// {
//     std::string allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~:/?#[]@!$&'()*+,;=";
//     if (this->currentHeaderKey.find_first_not_of(allowed) != std::string::npos)
//     {
//         this->status = 400;
//         return 0;
//     }
//     if (this->currentHeaderValue.find_first_not_of(allowed) != std::string::npos)
//     {
//         this->status = 400;
//         return 0;
//     }
//     return 1;
// }

int Request::readHeaders()
{
    if (this->_request.find("\r\n\r\n") == std::string::npos)
        return 1;
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
        this->currentHeaderKey = key;
        this->currentHeaderValue = value;
        this->_headers[key] = value;
    }
    if (this->TransferEncodingExists() && this->getTransferEncoding() != "chunked")
    {
        this->status = 501;
        return 0;
    }
    this->_request = this->_request.substr(this->_request.find("\r\n\r\n") + 4);
    
    if (this->_method != "POST")
    {
        std::cout << "#####" + this->_method << std::endl;
        this->_state = END;
        return 0;
    }
    if (this->isChunked())
    {
        this->_state = CHUNKED;
        this->_lengthState = 0;
    }
    else if (this->thereIsBoundary())
        this->_state = BOUNDARY;
    else if (this->ContentLengthExists())
    {
        this->_state = CONTENT_LENGTH;
        this->_lengthState = this->getContentLenght();
        this->_bodySize = this->_lengthState; 
    }
    else
    {
        this->status = 400;
        return 0;
    }
    return 1;
}

int Request::readByChunk()
{
    /*
       Read the request by chunk
    */
   switch (this->_chunkState)
   {
        case CHUNK_SIZE_START :
        {
            if (this->_request.find("\r\n") == std::string::npos)
                break;
            this->_chunkState = CHUNK_SIZE;
        }
        case CHUNK_SIZE :
        {
            if (this->_lengthState == 0)
            {
                std::string chunkSize = this->_request.substr(0, this->_request.find("\r\n"));
                std::stringstream ss(chunkSize);
                if(!(ss >> std::hex >> this->_lengthState))
                {
                    this->status = 400;
                    return 0;
                }
                if (this->_lengthState == 0)
                {
                    this->_state = END; 
                    return 0;
                }
                this->_bodySize += this->_lengthState;
                this->_request = this->_request.substr(this->_request.find("\r\n") + 2);
                this->_chunkState = CHUNK_DATA;
            }
            else
            {
                this->_chunkState = CHUNK_SIZE_START;
                return 1;
            }
        }
        case CHUNK_DATA :
        {
            if (this->_request.find("\r\n") == std::string::npos)
                return 1;
            std::ofstream file(this->_filename, std::ios::out | std::ios::app);
            if (this->_lengthState < this->_request.length())
            {
                file << this->_request.substr(0, this->_lengthState);
                this->_request = this->_request.substr(this->_lengthState + 2);
                this->_lengthState = 0;
                this->_chunkState = CHUNK_SIZE_START;
                file.close();
                return (readByChunk());
            }
            else if (this->_lengthState > 0)
            {
                if (this->_lengthState > this->_request.length())
                {
                    file << this->_request;
                    this->_lengthState -= this->_request.length();
                    this->_request = "";
                    file.close();
                    return 1;
                }
                else if (this->_lengthState == this->_request.length())
                {
                    file << this->_request;
                    this->_lengthState = 0;
                    this->_request = "";
                    file.close();
                    this->_chunkState = CHUNK_SIZE_START;
                    return 1;
                }
            }
        }
   }
    return 1;
}

int Request::readByContentLength()
{
    std::ofstream file(this->_filename, std::ios::out | std::ios::app);
    if (this->_request.length() > this->_lengthState)
    {
        file << this->_request.substr(0, this->_lengthState);
        this->_request = this->_request.substr(this->_lengthState);
        file.close();
        return 1;
    }
    this->_lengthState -= this->_request.length();
    if (this->_lengthState > 0)
    {
        file << this->_request;
        this->_request = "";
        file.close();
        return 1;
    }
    else if (this->_lengthState == 0)
    {
        file << this->_request;
        this->_request = "";
        file.close();
    }
    this->_state = END;
    return 0;
}

int Request::readBoundary()
{
    /*
        append the request to a file until the end of the request
    */
    if (this->_request.find(this->_boundary + "--") == std::string::npos)
    {
        std::ofstream file(this->_filename, std::ios::out | std::ios::app);
        this->_bodySize += this->_request.length();
        file << this->_request;
        this->_request = "";
        file.close();
        return 1;
    }
    else 
    {
        std::ofstream file(this->_filename, std::ios::out | std::ios::app);
        this->_bodySize += this->_boundary.length() + 2;
        file << this->_boundary + "--";
        this->_request = "";
        file.close();
        this->_state = END;
        return 0;
    }
}

int Request::parseRequest(char *buffer, int size, int fd)
{   
    (void)fd;
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
            std::getline(ss, this->_uri, ' ');
            std::getline(ss, this->_version, ' ');
            if (!validateRequestLine())
                goto end;
            this->_request = this->_request.substr(this->_request.find("\r\n") + 2);
            this->_state = HEADER;
        }
        case HEADER :
        {
            if (readHeaders())
                break ;
            goto end;
        }
        case CONTENT_LENGTH :
        {            
            if (readByContentLength())
                break ;
            goto end;
        }
        case CHUNKED :
        {
            if (readByChunk())
                break ;
            goto end;     
        }
        case BOUNDARY :
        {
            if (readBoundary())
               break ;
        }
        case END :
        {
            end:
                return 1;
        }
    }
    return 0;
}

void Request::printRequest()
{
    std::cout << "Method: " << this->_method << std::endl;
    std::cout << "Path: " << this->_uri << std::endl;
    std::cout << "Version: " << this->_version << std::endl;
    std::cout << "Headers: " << std::endl;
    for (std::map<std::string, std::string>::iterator it = this->_headers.begin(); it != this->_headers.end(); it++)
    {
        std::cout << it->first << ": " << it->second << std::endl;
    }
}
