#include "Response.hpp"

bool fileExists(const std::string& filename)
{
    std::ifstream file(filename.c_str());
    return file.good();
}

std::string generateNewFileName(const std::string& filename)
{
    size_t dotPos = filename.find_last_of('.');
    std::string baseName, extension;

    if (dotPos != std::string::npos) {
        baseName = filename.substr(0, dotPos);
        extension = filename.substr(dotPos);
    } else {
        baseName = filename;
        extension = "";
    }

    for (int i = 1; ; ++i) {
        std::stringstream ss;
        ss << baseName << "_" << i << extension;
        std::string newFileName = ss.str();

        if (!fileExists(newFileName)) {
            return newFileName;
        }
    }
}

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

bool    Response::normalUpload(Request &request)
{
    char    buffer[10000];

    if (this->index == 0)
    {    
        std::string      name;
        std::string      extension;

        name = request.getFilename().substr(request.getFilename().find_last_of("/") + 1);
        extension = getExtension(request.getContentType());
        if (extension != "")
            this->fileToUpload = this->location->getUploadLocation() + "/" + name + "." + extension;
        else
            this->fileToUpload = this->location->getUploadLocation() + "/" + name;
        this->index++;
    }
   
    std::ifstream inputfile(request.getFilename().c_str(), std::ios::in);
    if (!inputfile.is_open()) {
        throw 404;
    }
    
    std::ofstream outputfile(this->fileToUpload.c_str(), std::ios::out | std::ios::app);
    if (!outputfile.is_open())
    {
        inputfile.close();
        throw 404;
    }

    inputfile.seekg(this->fileOffset);
    inputfile.read(buffer, 10000);
    outputfile.write(buffer, inputfile.gcount());
    if (outputfile.fail())
    {
        inputfile.close();
        outputfile.close();
        throw 507;
    }
    this->fileOffset += inputfile.gcount();
    if (this->fileOffset == request.getBodysize())
    {
        this->index = 0;
        this->fileOffset = 0;
        inputfile.close();
        outputfile.close();

        this->status = 201;
        this->headers["Location: "] = this->fileToUpload;
        this->headers["Content-Length: "] = "0";
        send_status_line_and_headers();
        return true;
    }
    inputfile.close();
    outputfile.close();
    return false;
}

bool    Response::uploadWithBoundary(Request &request)
{
    std::string boundary = "--" + request.getBoundary();
    if (this->index == 0) {
        char        buffer[10000];
        std::fstream inputfile(request.getFilename().c_str(), std::ios::in);
        if (!inputfile.is_open())
            throw 404;
        inputfile.seekg(this->fileOffset);
        inputfile.read(buffer, 10000);
        this->fileOffset += inputfile.gcount();
        this->bodyReaded += std::string(buffer, inputfile.gcount());
        this->index++;
        inputfile.close();
    }

    if (this->boundaryState == START_BOUNDARY) {
        size_t  pos = this->bodyReaded.find(boundary);
        if (pos != std::string::npos) {
            this->boundaryState = HEADER_BOUNDARY;
            this->bodyReaded = this->bodyReaded.substr(pos + boundary.length() + 2);
        }
    }

    if (this->boundaryState == HEADER_BOUNDARY) {
        std::string headers;
        size_t  pos = this->bodyReaded.find("\r\n\r\n");
        if (pos != std::string::npos)
        {
            headers = this->bodyReaded.substr(0, pos);
            this->bodyReaded = this->bodyReaded.substr(pos + 4);
            std::string name;
            size_t      posf = headers.find("filename=\"");
            if (posf != std::string::npos)
                name = headers.substr(posf + 10);
            else
                name = headers.substr(headers.find("name=\"") + 6);
            name = name.substr(0, name.find("\""));
            this->fileToUpload = this->location->getUploadLocation() + "/" + name;
            if (fileExists(this->fileToUpload))
                this->fileToUpload = generateNewFileName(this->fileToUpload);
            this->boundaryState = MIDDLE_BOUNDARY;
        }
    }

    if (this->boundaryState == MIDDLE_BOUNDARY) {
        std::ofstream outputfile(this->fileToUpload.c_str(), std::ios::out | std::ios::app);
        if (!outputfile.is_open())
        {
            outputfile.close();
            throw 404;
        }
        size_t  pos = this->bodyReaded.find(boundary);
        if (pos != std::string::npos) {
            std::string body = this->bodyReaded.substr(0, pos - 2);
            this->bodyReaded = this->bodyReaded.substr(pos + boundary.length());
            if (this->bodyReaded == "--\r\n")
            {
                outputfile << body;
                if (outputfile.fail())
                {
                    outputfile.close();
                    throw 507;
                }
                this->bodyReaded = "";
                this->index = 0;
                outputfile.close();
                this->status = 201;
                this->headers["Location: "] = this->fileToUpload;
                this->headers["Content-Length: "] = "0";
                send_status_line_and_headers();
                return true;
            }
            outputfile << body;
            if (outputfile.fail())
            {
                outputfile.close();
                throw 507;
            }
            this->boundaryState = HEADER_BOUNDARY;
            this->index = 0;
            outputfile.close();
            return (uploadWithBoundary(request));
        }
        else
        {
            outputfile << this->bodyReaded;
            if (outputfile.fail())
            {
                outputfile.close();
                throw 507;
            }
            this->bodyReaded = "";
            this->index = 0;
        }
        outputfile.close();
    }
    return false;
}

bool    Response::uploadPostMethod(Request &request)
{
    if (!request.thereIsBoundary())
        return normalUpload(request);
    else
        return uploadWithBoundary(request);
}
