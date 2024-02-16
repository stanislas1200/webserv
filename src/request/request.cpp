#include "../../include/Webserv.hpp"


void handleGetRequest(int connection, s_request request) {
	
	std::string path = request.path;
	if (path == "/") {
		path = "/index.html";
	}
	std::string fullPath = "src/pages" + path; // src/pages/../main.cpp
	std::ifstream file(fullPath.c_str(), std::ios::binary);
	if (!file.is_open()) {
		std::string response = "HTTP/1.0 404 Not Found\r\nContent-type:text/html\r\n\r\n";
		std::string page = "<html>\n<head>\n<title>404 Not Found</title>\n</head>\n<body>\n<h2>404 Not Found</h2>\n</body>\n</html>\n\n";
		send(connection, response.c_str(), response.length(), 0);
		send(connection, page.c_str(), page.length(), 0);
		error("File:", strerror(errno), fullPath.c_str());
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

s_FormDataPart getFormData(int connection, s_request request) {
	s_FormDataPart formData;
	size_t bytes = 0;
	size_t pos = 0;
	pos = request.headers["Content-Type"].find("boundary=") + 9;
	std::string boundary = "--" + request.headers["Content-Type"].substr(pos, pos + 24);
	size_t dataLen = 0;
	std::string header;
	char buffer[1024];
	std::cout << C << std::endl;
	// read on socket
	while ((bytes = recv(connection, buffer, 1000, 0)) > 0)
	{
		if (bytes > 1000) // 18446744073709551615 // non blocking socket
			break;
		if ((pos = header.find("\r\n\r\n")) == std::string::npos)
			header += buffer;
		dataLen += bytes;
		formData.data.insert(formData.data.end(), buffer, buffer + bytes);		
	}

	// parse header
	std::istringstream iss(header);
	std::string line;
	while (std::getline(iss, line) && !line.empty())
	{
		if (line == "\r")
			break;
		if (line.find("Content-Disposition") != std::string::npos)
		{
			line = line.substr(line.find("name=\"") + 6);
			formData.name = line.substr(0, line.find('"'));
			line = line.substr(line.find("filename=\"") + 10);
			formData.filename = line.substr(0, line.find('"'));
		}
		if (line.find("Content-Type") != std::string::npos)
			formData.contentType = line.substr(line.find("Content-Type:") + 14);
	}

	// write in file
	std::ofstream outputFile(("upload/" + formData.filename).c_str(), std::ios::binary);
	if (!outputFile.is_open())
			error("Open:", strerror(errno), NULL);
	else
		std::cout << GREEN << MB "File uploaded!" << std::endl;
	outputFile.write(&formData.data[pos + 4], dataLen - (pos + 4) - boundary.size()); // +2 if end request (--)
	outputFile.close();
	handleGetRequest(connection, request);
	return formData;
} 

void handlePostRequest(int connection, s_request request) {
	if (request.method == "POST" && request.headers["Content-Type"].find("multipart/form-data") != std::string::npos) {
		s_FormDataPart formData = getFormData(connection, request);
	}
	send(connection, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"), 0);
	return;
}

void handleDeleteRequest(int connection, s_request request) {
	if (std::remove(("upload/" + request.path).c_str()))
		error("DELETE:", "no file:", ("upload/" + request.path).c_str());
	send(connection, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"), 0);
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
	
	if (request.method == "GET")
		handleGetRequest(connection, request);
	else if (request.method == "POST")
		handlePostRequest(connection, request);
	else if (request.method == "DELETE")
		handleDeleteRequest(connection, request);
	else
	{
		std::string response = "HTTP/1.0 505 Not supported\r\nContent-type:text/html\r\n\r\n";
		send(connection, response.c_str(), response.length(), 0);
	}
}
