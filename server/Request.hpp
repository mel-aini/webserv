/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hel-mamo <hel-mamo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/23 11:50:56 by hel-mamo          #+#    #+#             */
/*   Updated: 2023/12/24 16:07:57 by hel-mamo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# ifndef Request_HPP
# define Request_HPP

# include <iostream>
# include <string>
# include <map>
# include <vector>
# include <algorithm>
# include <iterator>
# include <sstream>
# include <fstream>

enum State{
    START,
    METHOD,
    HEADER,
    CONTENT_LENGTH,
    CHUNKED,
    BOUNDARY,
    END
};

enum Chunk_State{
    CHUNK_SIZE_START,
    CHUNK_SIZE,
    CHUNK_DATA,
};


class Request
{
    private:
        int             status;
        std::string     _fd;
        State           _state;
        Chunk_State     _chunkState;
        size_t          _lengthState;
        std::string     _request;
        std::string     _method;
        std::string     _uri;
        std::string     _version;
        std::string     currentHeaderKey;
        std::string     currentHeaderValue;
        std::map        <std::string, std::string> _headers;
        std::string     _filename;
        std::string     _boundary;

    public:
        Request();
        Request(Request const &src);
        Request     &operator=(Request const &rhs);
        int         parseRequest(char *buffer, int size, int fd);
        void        printRequest();
        size_t      getContentLenght();
        std::string getTransferEncoding();
        bool        ContentLengthExists();
        bool        TransferEncodingExists();
        bool        isChunked();
        int         readByChunk();
        int         readByContentLength();
        int         readHeaders();
        int         readBoundary();
        int         validateRequestLine();
        int         validateHeaderLine();
        int         thereIsBoundary();
};


#endif