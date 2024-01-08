#pragma once
#include <iostream>
#include <map>
#include "Location.hpp"
#include "Request.hpp"
#include <sstream>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include "../public/Colors.hpp"
#include "HtmlTemplate.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

bool    hasQueryString(std::string uri);

enum METHOD {
	GET,
	POST,
};

enum sending_level{
    GET_REQUESTED_RES,
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
        size_t                              fileOffset;
        std::map<int, std::string>          status_codes;
        int                                 socket;
        int                                 index;
        bool                                sendingFile;
        std::string                         errPage;
        struct stat                         fileInf;
        std::map<std::string, std::string>  content_type;
        std::string                         fileToSend;
        std::string                         fileToUpload;
        std::string                         bodyFileName;

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
        void                setError(int status_code);
        void                setSendingLevel(unsigned int level);
        void                setBodyFileName(std::string bodyFileName);

		bool                            send_response_error();
        bool                            send_response_index_files(std::string uri);
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
        bool                            newGet(std::string uri, std::map <std::string, std::string> firstCgiEnv, int method_type);
        bool                            getRequestedResource(std::string uri);
        std::pair<std::string, size_t>  getMatchIndex(std::string uri);
        bool                            readAndSendFile(std::string path, size_t size);
        std::string                     getContentType(std::string path);
        bool                            getRequestedFile(std::string uri);
        bool                            isFileExist(std::string& target);
        bool                            isTarget(std::string& target, struct stat *fileInfo);
        void	                        log();
        void                            log_res_type();
        void                            log_res_level();
        void                            log_members();
        bool                            uploadPostMethod(Request &request);
        std::string                     getExtension(std::string filename);
        char**				            getCgiEnv(int method_type, std::string uri, std::map <std::string, std::string> firstCgiEnv);
        void                            executeCgi(std::string uri, std::map <std::string, std::string> firstCgiEnv, int method_type);
        class ResponseFailed : public std::exception {
			public:
				const char * what() const throw();
		};
};
//4096 page memery