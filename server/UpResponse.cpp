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
            this->fileToUpload = "./" + this->location->getUploadLocation() + "/" + name + "." + getExtension(request.getContentType());
            this->index++;
        }
        std::ifstream inputfile(request.getFilename().c_str(), std::ios::in);
        if (!inputfile.is_open())
            throw 404;
        
        std::ofstream outputfile(this->fileToUpload.c_str(), std::ios::out | std::ios::app);
        if (!outputfile.is_open())
            throw 404;

        inputfile.seekg(this->fileOffset);
        inputfile.read(buffer, 10000);
        outputfile.write(buffer, inputfile.gcount());
        this->fileOffset += inputfile.gcount();
        if (this->fileOffset == request.getBodysize())
        {
            this->index = 0;
            this->status = 201;
            this->fileOffset = 0;
            inputfile.close();
            outputfile.close();
            return true;
        }
        inputfile.close();
        outputfile.close();
    }
    else
    {
        // read line by line
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
            name = headers.substr(headers.find("filename=\"") + 10);
            name = name.substr(0, name.find("\""));
            this->fileToUpload = "./" + this->location->getUploadLocation() + "/" + name; 
        }
        std::ofstream outputfile(this->fileToUpload.c_str(), std::ios::out | std::ios::app);
        if (!outputfile.is_open())
            throw 404;
        
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
            std::cout << "201 : " << line << std::endl;
            this->index = 0;
            this->status = 201;
            inputfile.close();
            outputfile.close();
            return true;
        }
        this->index = 0;
        inputfile.close();
        outputfile.close();
    }
    return false;
}
