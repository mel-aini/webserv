#pragma once
#include <iostream>
#include <map>
#include "Location.hpp"
#include <sstream>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include "../public/Colors.hpp"
#include "HtmlTemplate.hpp"
#include <sys/stat.h>
#include <dirent.h>

enum sending_level{
    SENDING_HEADERS,
    SENDING_BODY,
    SENDING_END,
};

enum method_level{
    FINDRESOURCE,
    DATA_SENDING,
    DATA_SEND,
};

enum request_case{
    OTHER_CASE,
    DIR_CASE,
    FILE_CASE,
    NO_CASE,
};

enum match_index{
    YES,
    NO,
};

enum response_state {
    OK,
    ERROR,
    REDIRECT,
};

class Response {
    private:
        unsigned int                        status;
        std::string                         message;
        std::string                         body;
        std::map<std::string, std::string>  headers;
        Location                            *location;
        unsigned int                        sending_level;
        unsigned int                        method_level;
        unsigned int                        request_case;
        unsigned int                        response_type;
        unsigned int                        match_index;
        size_t                              bodyOffset;
        std::map<int, std::string>          status_codes;
        int                                 socket;
        bool                                sendingFile;
        std::string                         errPage;
        struct stat                         fileInf;
        std::map<std::string, std::string>  content_type;

    public:
        Response();
        ~Response();

        const std::string&  getBody() const;
        int                 getStatus() const;
        void                setStatus(unsigned int status);
        std::string         getStatusMessage();
        unsigned int        getResponseType() const;
        unsigned int        getSendingLevel() const;
        void                setSocket(int fd);

        void                setSendingLevel(unsigned int level);

		bool                            send_response_error();
        bool                            send_response_index_files(std::string path, std::vector<std::string> content);
        void                            send_status_line_and_headers();
        void                            send_body();
        void                            setResponseType(unsigned int response_type);
        Location                        *findLocation(std::vector<Location> &locations, std::string uri);
        void                            setLocation(Location *location);
        Location                        *getLocation();
        bool                            isInErrorPages();
        void                            redirect(const std::string& location);
        void                            reset();
        bool                            sendFile(std::string fileName);
        bool                            getMethod(std::string uri, char **env);
        bool                            getRequestedResource(std::string uri);
        std::pair<std::string, size_t>  getMatchIndex(std::string uri);
        bool                            readAndSendFile(std::string path, size_t size);
        std::string                     getContentType(std::string path);
        void	                        log();
    
        class ResponseFailed : public std::exception {
			public:
				const char * what() const throw();
		};
};