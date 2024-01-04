#include "Response.hpp"

bool    Response::uploadPostMethod(Request &request)
{
    std::string      path;
    std::string      body;
    std::string      boundary;
    std::string      filename;
    std::string      extension;
    char             buffer[10000];

    std::cout << "uploadPostMethod" << std::endl;
    if (!request.thereIsBoundary())
    {
        extension = request.getContentType();
        std::replace(extension.begin(), extension.end(), '/', '.');
        filename = "./" + this->location->getUploadLocation() + "/" + extension;
        std::cout << "f" << this->location->getUploadLocation() << std::endl;
        std::cout << "filename: " << filename << std::endl;
        std::ifstream inputfile(request.getFilename().c_str(), std::ios::in);
        if (!inputfile.is_open())
        {
            // todo: throw 404 instead
            this->status = 404;
            return false;
        }
        std::ofstream outputfile(filename.c_str(), std::ios::out);
        if (!outputfile.is_open())
        {
            // todo: throw 404 instead
            this->status = 404;
            perror("open");
            return false;
        }
        while (inputfile.read(buffer, sizeof(buffer)))
            outputfile.write(buffer, sizeof(buffer));
        inputfile.close();
        outputfile.close();
    }
    else
    {
       std::ifstream inputfile(request.getFilename().c_str(), std::ios::in);
        if (!inputfile.is_open())
        {
            // todo: throw 404 instead
            this->status = 404;
            std::cout << "file not open" << std::endl;
            return false;
        }
        while (std::getline(inputfile, body, '\n'))
        {
            if (body.find(request.getBoundary()) != std::string::npos)
                continue;
        }
    }
    return true;
}
