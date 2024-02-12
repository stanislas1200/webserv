#include "../../include/Webserv.hpp"


void handleGetRequest(int connection, s_request request) {
	
	std::string path = request.path;
	if (path == "/") {
		path = "/index.html";
	}
	std::string fullPath = "src/pages" + path; // src/pages/../main.cpp
	std::ifstream file(fullPath.c_str(), std::ios::binary);
	if (!file.is_open()) {
		error("File:", strerror(errno), NULL);
		std::string response = "HTTP/1.0 404 Not Found\r\nContent-type:text/html\r\n\r\n";
		std::string page = "<html>\n<head>\n<title>404 Not Found</title>\n</head>\n<body>\n<h2>404 Not Found</h2>\n</body>\n</html>\n\n";
		send(connection, response.c_str(), response.length(), 0);
		send(connection, page.c_str(), page.length(), 0);
		return ;
	}

	send(connection, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"), 0);
	send(connection, "Content-Type: text/html\r\n\r\n", strlen("Content-Type: text/html\r\n\r\n"), 0);

	// Send file content
	char buffer[1024];
	while (file.read(buffer, sizeof(buffer)).gcount() > 0) {
		send(connection, buffer, file.gcount(), 0);
	}

	file.close();
}

std::vector<s_FormDataPart> parseMultipartFormData(const std::string& body, const std::string& boundary) {
	std::vector<s_FormDataPart> parts;
	// Find the boundary delimiter
    std::string delimiter = "--" + boundary;
    size_t pos = body.find(delimiter);
    if (pos == std::string::npos)
        return parts; // No parts found

    // Iterate through each part
    do {
        s_FormDataPart part;

        // Find the end of the headers
        size_t headerEnd = body.find("\r\n\r\n", pos);
        if (headerEnd == std::string::npos)
            break; // Malformed request

        // Parse headers
        std::string headers = body.substr(pos + delimiter.length(), headerEnd - (pos + delimiter.length()));
        size_t namePos = headers.find("name=\"");
        size_t filenamePos = headers.find("filename=\"");
        if (namePos != std::string::npos) {
            size_t nameEnd = headers.find("\"", namePos + 6);
            part.name = headers.substr(namePos + 6, nameEnd - (namePos + 6));
			std::cout << RED "NAME: " MB << part.name << C << std::endl;
        }
        if (filenamePos != std::string::npos) {
            size_t filenameEnd = headers.find("\"", filenamePos + 10);
            part.filename = headers.substr(filenamePos + 10, filenameEnd - (filenamePos + 10));
        }
        part.contentType = "text/plain"; // Default content type
        if (filenamePos != std::string::npos) {
            size_t contentTypePos = headers.find("Content-Type: ");
            if (contentTypePos != std::string::npos) {
                size_t contentTypeEnd = headers.find("\r\n", contentTypePos + 14);
                part.contentType = headers.substr(contentTypePos + 14, contentTypeEnd - (contentTypePos + 14));
            }
        }

        // Find the start of the data
        size_t dataStart = headerEnd + 4;
        if (dataStart >= body.length())
		{
			std::cout << RED "MALFORMED REQUEST\n" C;
            break; // Malformed request
		}

        // Find the end of the part
        size_t partEnd = body.find(delimiter, dataStart);
        if (partEnd == std::string::npos)
		{
			std::cout << RED "MALFORMED REQUEST\n" C;
			break; // Malformed request
		}

        // Extract data
        part.data.assign(body.begin() + dataStart, body.begin() + partEnd - 2); // Subtract 2 to exclude \r\n at the end

        parts.push_back(part);

        // Move to the next part
        pos = body.find(delimiter, partEnd);
    } while (pos != std::string::npos);

    return parts;
}

void handlePostRequest(int connection, s_request request) {
	std::cout << "POST REQUEST\n";
	if (request.method == "POST" && request.headers["Content-Type"].find("multipart/form-data") != std::string::npos) {
		std::string dataHeader;
		size_t bytes = 0;
		char buffer[1024];
		while ((bytes = recv(connection, buffer, 1, 0)) > 0) // 
		{
			buffer[bytes] = '\0';
			dataHeader += buffer;
			if (dataHeader.find("\r\n\r\n") != std::string::npos)
				break;
		}
		std::string data;
		std::vector<char> binary;
		size_t pos;
		while ((bytes = recv(connection, buffer, 1, 0)) > 0)
		{
			buffer[bytes] = '\0';
			data += buffer;
			binary.push_back(buffer[0]);
			if ((pos = data.find("------WebKitFormBoundary")) != std::string::npos)
			{
				// binary.erase(binary.begin() + pos, binary.end());
				break;
			}
		}
		std::ofstream outputFile("output.png", std::ios::binary);
		if (!outputFile.is_open())
				error("Open:", strerror(errno), NULL);
		outputFile.write(&binary[0], binary.size());
		outputFile.close();
	}
	send(connection, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"), 0);
	return;
}

void handleDeleteRequest(int connection, std::string request) {
	(void)connection;
	(void)request;
}

void printRequest(s_request request) {
	std::cout << GREEN "Method: " MB << request.method << std::endl;
	std::cout << GREEN "Path: " MB << request.path << std::endl;
	std::map<std::string, std::string>::const_iterator it;
	for (it = request.headers.begin(); it != request.headers.end(); ++it) {
		std::cout << MB << it->first << C ": " DV << it->second << std::endl;
	}
	std::cout << YELLOW "Body:\n" MB << request.body << std::endl;
}

void parseRequest(int connection, std::string header) {
	s_request request;
	std::istringstream requestStream(header);
	std::string line;

	// Parse the first line of the request
	std::getline(requestStream, line);
	std::istringstream firstLineStream(line);
	firstLineStream >> request.method >> request.path;

	// Parse the headers
	while (std::getline(requestStream, line) && line != "\r") {
		std::istringstream headerStream(line);
		std::string headerName;
		std::string headerValue;
		std::getline(headerStream, headerName, ':');
		std::getline(headerStream, headerValue);
		request.headers[headerName] = headerValue;
	}

	// // Parse the body
	// std::stringstream ss;
	// ss << requestStream.rdbuf();
	// request.body = ss.str();

	printRequest(request);
	
	if (request.method == "GET")
		handleGetRequest(connection, request);
	else if (request.method == "POST")
		handlePostRequest(connection, request);
	else if (method == "DELETE")
		handleDeleteRequest(connection, request);
	else
	{
		std::string response = "HTTP/1.0 505 Not supported\r\nContent-type:text/html\r\n\r\n";
		send(connection, response.c_str(), response.length(), 0);
	}
}
