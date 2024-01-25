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
        std::cout << request.getFilename() << std::endl;
        throw 404;
    }
    
    std::ofstream outputfile(this->fileToUpload.c_str(), std::ios::out | std::ios::app);
    if (!outputfile.is_open())
    {
        std::cout << this->fileToUpload<< std::endl;
        inputfile.close();
        unlink(request.getFilename().c_str());
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
        unlink(request.getFilename().c_str());

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
        this->fileOffset += headers.length() + 2;
        std::string name;
        size_t      pos = headers.find("filename=\"");
        if (pos != std::string::npos)
            name = headers.substr(pos + 10);
        else
            name = headers.substr(headers.find("name=\"") + 6);
        name = name.substr(0, name.find("\""));
        this->fileToUpload = this->location->getUploadLocation() + "/" + name;
        if (fileExists(this->fileToUpload))
            this->fileToUpload = generateNewFileName(this->fileToUpload);
    }
    std::ofstream outputfile(this->fileToUpload.c_str(), std::ios::out | std::ios::app);
    if (!outputfile.is_open())
    {
        inputfile.close();
        unlink(request.getFilename().c_str());
        throw 404;
    }

    int i = 0;
    while (std::getline(inputfile, line))
    {
        if (line.find(boundary) != std::string::npos || i > 1000)
            break ;
        outputfile << line + "\n";
        if (outputfile.fail())
        {
            inputfile.close();
            outputfile.close();
            throw 507;
        }
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
        inputfile.close();
        outputfile.close();
        unlink(request.getFilename().c_str());

        this->status = 201;
        this->headers["Location: "] = this->fileToUpload;
        this->headers["Content-Length: "] = "0";
        send_status_line_and_headers();
        return true;
    }
    this->index = 0;
    inputfile.close();
    outputfile.close();
    return false;
}

bool    Response::uploadPostMethod(Request &request)
{
    if (!request.thereIsBoundary())
        return normalUpload(request);
    else
        return uploadWithBoundary(request);
}
