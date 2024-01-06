#include "Response.hpp"

bool    Response::uploadPostMethod(Request &request)
{
    std::string      filename;
    std::string      extension;
    char             buffer[10000];

    if (!request.thereIsBoundary())
    {
        extension = request.getContentType();
        std::replace(extension.begin(), extension.end(), '/', '.');
        filename = "./" + this->location->getUploadLocation() + "/" + extension;
        std::ifstream inputfile(request.getFilename().c_str(), std::ios::in);
        if (!inputfile.is_open())
        {
            this->status = 404;
            return false;
        }
        std::ofstream outputfile(filename.c_str(), std::ios::out | std::ios::app);
        if (!outputfile.is_open())
        {
            this->status = 404;
            perror("open");
            return false;
        }
        inputfile.seekg(this->fileOffset);
        inputfile.read(buffer, 10000);
        outputfile.write(buffer, inputfile.gcount());
        this->fileOffset += inputfile.gcount();
        std::cout << "fileOffset: " << this->fileOffset << std::endl;
        std::cout << "bodysize: " << request.getBodysize() << std::endl;
        if (this->fileOffset == request.getBodysize())
        {
            this->status = 201;
            this->fileOffset = 0;
            inputfile.close();
            outputfile.close();
            return true;
        }
        inputfile.close();
        outputfile.close();
    }
    return false;
}
