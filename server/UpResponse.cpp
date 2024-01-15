#include "Response.hpp"

std::string    Response::getExtension(std::string filename)
{
    std::string extension;
    size_t      pos;

    pos = filename.find_last_of("/");
    if (pos != std::string::npos)
        extension = filename.substr(pos + 1);
    else
        extension = "";
    return extension;
}

bool    Response::uploadPostMethod(Request &request)
{
    char             buffer[10000];

    if (!request.thereIsBoundary())
    {
        if (this->index == 0)
        {    
            std::string      name;

            name = request.getFilename().substr(request.getFilename().find_last_of("/") + 1);
            this->fileToUpload = this->location->getUploadLocation() + "/" + name + "." + getExtension(request.getContentType());
            this->index++;
        }
        std::ifstream inputfile(request.getFilename().c_str(), std::ios::in);
        if (!inputfile.is_open())
            throw 404;
        
        std::ofstream outputfile(this->fileToUpload.c_str(), std::ios::out | std::ios::app);
        if (!outputfile.is_open())
        {
            unlink(request.getFilename().c_str());
            throw 404;
        }

        inputfile.seekg(this->fileOffset);
        inputfile.read(buffer, 10000);
        outputfile.write(buffer, inputfile.gcount());
        this->fileOffset += inputfile.gcount();
        if (this->fileOffset == request.getBodysize())
        {
            this->index = 0;
            this->fileOffset = 0;
            inputfile.close();
            outputfile.close();
            unlink(request.getFilename().c_str());

            this->status = 201;
            this->headers["Location: "] = this->fileToUpload;
            this->headers["Content-Length: "] = "0";
            send_status_line_and_headers();
            return true;
        }
        inputfile.close();
        outputfile.close();
    }
    else
    {
        std::string boundary = "--" + request.getBoundary();
        std::string line;
        std::fstream inputfile(request.getFilename().c_str(), std::ios::in);
        if (!inputfile.is_open())
            throw 404;

        inputfile.seekg(this->fileOffset);
        if (this->index == 0)
        {
            std::getline(inputfile, line);
            this->fileOffset += line.length() + 1;
            line += "\n";
            std::string headers;
            while (std::getline(inputfile, line))
            {
                if (line == "\r")
                    break;
                headers += line + "\n";
            }
            this->fileOffset += headers.size() + 2;
            std::string name;
            size_t      pos = headers.find("filename=\"");
            if (pos != std::string::npos)
                name = headers.substr(pos + 10);
            else
                name = headers.substr(headers.find("name=\"") + 6); 
            name = name.substr(0, name.find("\""));
            this->fileToUpload = this->location->getUploadLocation() + "/" + name;
            std::cout << "##### file: " + this->fileToUpload << std::endl;
        }
        std::ofstream outputfile(this->fileToUpload.c_str(), std::ios::out | std::ios::app);
        if (!outputfile.is_open())
        {
            inputfile.close();
           // unlink(request.getFilename().c_str());
            throw 404;
        }
        
        int i = 0;
        while (std::getline(inputfile, line))
        {
            if (line.find(boundary) != std::string::npos || i > 1000)
                break ;
            outputfile << line + "\n";
            this->fileOffset += line.length() + 1;
            i++;
        }
        if (i > 1000)
        {
            inputfile.close();
            outputfile.close();
            this->index = 1;
            return false;
        }
        if (line.find(boundary + "--") != std::string::npos)
        {
            this->index = 0;
            this->fileOffset += line.length() + 1;
            // std::cout << "-->" << request.getBodysize() << std::endl;
            // std::cout << "-->" <<  this->fileOffset << std::endl;
            inputfile.close();
            outputfile.close();
            // unlink(request.getFilename().c_str());

            this->status = 201;
            this->headers["Location: "] = this->fileToUpload;
            this->headers["Content-Length: "] = "0";
            send_status_line_and_headers();
            return true;
        }
        this->index = 0;
        inputfile.close();
        outputfile.close();
    }
    return false;
}
