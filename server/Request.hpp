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
# include <sys/time.h>
# include "../public/Colors.hpp"

enum State {
    START,
    METHOD,
    HEADER,
    CONTENT_LENGTH,
    CHUNKED,
    BOUNDARY,
    END
};

enum Chunk_State {
    CHUNK_SIZE_START,
    CHUNK_SIZE,
    CHUNK_DATA,
};


class Request
{
    private:
        int             status;
        size_t          _bodySize;
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
        ~Request();
        Request(Request const &src);
        Request     &operator=(Request const &rhs);
        bool        parseRequest(char *buffer, int size);
        bool        isAssci(std::string str);
        int         parseMethod();
        long        toLong(std::string str);
        void        printRequest();
        size_t      getContentLenght();
        std::string getTransferEncoding();
        bool        ContentLengthExists();
        bool        TransferEncodingExists();
        bool        isChunked();
        bool        isHostExists();
        int         readByChunk();
        int         readByContentLength();
        int         readHeaders();
        int         readBoundary();
        int         validateRequestLine();
        int         validateHeaderLine();
        int         thereIsBoundary();
        int         getReadingMethod();

        std::string getHeader(std::string key);
        int         getStatus();
        std::map<std::string, std::string> getHeaders();
        std::string getMethod();
        std::string getUri();
        void        setUri(std::string str);
        std::string getVersion();
        std::string getFilename();
        std::string getBoundary();
        std::string getContentType();
        size_t      getBodysize();
        State       getState() const;
        std::string getHeaderLine(std::string key);
        std::string GenerateName();
        void        reset();
		void		log_uri();

        void log();

        class RequestFailed : public std::exception {
			public:
				const char * what() const throw();
		};
};

void            skipSlash(std::string & str);
int	            isNum(std::string str);
std::string     trimSpacesAndTabs(std::string str);

#endif