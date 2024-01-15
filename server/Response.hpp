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
#include "Cgi.hpp"
#include "Log.hpp"

enum sending_level{
    GET_REQUESTED_RES,
    SENDING_HEADERS,
    SENDING_BODY,
    SENDING_END,
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
        Cgi                                 cgi;
        Log                                 traces;

    public:
        Response();
        ~Response();

        // title: Getters
        Log&                            getTraces();
        int                             getStatus() const;
        int                             getSocket();
        void                            setStatus(unsigned int status);
        std::string                     getStatusMessage();
        unsigned int                    getResponseType() const;
        unsigned int                    getSendingLevel() const;
        Location                        *getLocation();

        // title: Setters
        void                            setSocket(int fd);
        void                            setSendingLevel(unsigned int level);
        void                            setBodyFileName(std::string bodyFileName);

        // title: Methods
        bool                            getRequestedResource(std::string uri);
		bool                            send_response_error();
        bool                            send_response_index_files(std::string uri);
        void                            send_status_line_and_headers();
        void                            setResponseType(unsigned int response_type);
        void                            setLocation(Location *location);
        bool                            isInErrorPages();
        void                            redirect(const std::string& location);
        bool                            sendFile(std::string fileName);
        bool                            get_method(std::string uri, std::map <std::string, std::string> firstCgiEnv);
        bool                            post_method(Request &request, std::map <std::string, std::string> _headers);
        bool                            uploadPostMethod(Request &request);
        bool                            delete_method(std::string uri);
        void                            check_dir_permission(std::string target);
        void                            remove_dir(std::string target);
        void                            decode_uri(std::string& uri);
        std::string                     getContentType(std::string path);
        bool                            isFileExist(std::string& target);
        bool                            isTarget(std::string& target, struct stat *fileInfo);
        std::string                     getExtension(std::string filename);
        bool                            hasCgi(void);
        void                            reset();
        

        // title: exceptions
        class ResponseFailed : public std::exception {
			public:
				const char * what() const throw();
		};

        class ConnectionClosed : public std::exception {
			public:
				const char * what() const throw();
		};


        // title: log
        void                            log_res_type();
        void                            log_res_level();
        void                            log_members();
        void                            log_response();
};
//4096 page memery