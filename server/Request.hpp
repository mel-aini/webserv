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

enum ChunkState{
    CHUNK_SIZE_START,
    CHUNK_SIZE,
    CHUNK_DATA,
    CHUNK_END
};


class Request
{
    private:
        int     status;
        State _state;
        ChunkState _chunkState;
        size_t _lengthState;
        std::string _request;
        std::string _method;
        std::string _url;
        std::string _version;
        std::string currentHeaderKey;
        std::string currentHeaderValue;
        std::map<std::string, std::string> _headers;
        std::string _filename;
    public:
        Request();
        Request(Request const &src);
        Request &operator=(Request const &rhs);
        int readRequest(char *buffer, int size);
        void printRequest();
        size_t getContentLenght();
        std::string getTransferEncoding();
        bool ContentLengthExists();
        bool TransferEncodingExists();
        bool isChunked();
        int readByChunk();
        
};


#endif