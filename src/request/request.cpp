#include "../../include/Webserv.hpp"

void handleGetRequest(int connection, s_request request) {
	
	std::string path = request.path;
	// TODO : CGI
	if (path == "/") {
		path = "/index.html";
	}

	std::string fullPath = "src/pages" + path; // src/pages/../main.cpp
	std::ifstream file(fullPath.c_str(), std::ios::binary);
	std::string status = "200";

	if (!file.is_open()) {
		status = "404"; // sendfile handle error; make a class ?
		file.open("src/pages/errorpages/error_404.html", std::ios::binary);
		error("File:", strerror(errno), fullPath.c_str());
	}
	
	sendFile(connection, &file, status);
	return;
}

std::string handleFormData(int connection, s_request request) {
	s_FormDataPart formData;
	size_t bytes = 0;
	size_t pos = 0;
	pos = request.headers["Content-Type"].find("boundary=") + 9;
	std::string boundary = "--" + request.headers["Content-Type"].substr(pos, pos + 24);
	size_t dataLen = 0;
	std::string header;
	char buffer[10024];
	std::cout << C << std::endl;
	// read on socket // TODO : handle multiple files
	while ((bytes = recv(connection, buffer, 10000, 0)) > 0 && bytes != std::string::npos)
	{
		if ((pos = header.find("\r\n\r\n")) == std::string::npos)
			header += buffer;
		
		dataLen += bytes;
		formData.data.insert(formData.data.end(), buffer, buffer + bytes);		

		if (stoul(request.headers["Content-Length"]) == formData.data.size())
			break;
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
	// TODO : check malformated request
	// write in file
	std::ofstream outputFile(("upload/" + formData.filename).c_str(), std::ios::binary);
	if (!outputFile.is_open())
	{
		error("Open:", strerror(errno), NULL);
		send(connection, "File upload failed! ", strlen("File upload failed! "), 0);
		return "500";
	}
	std::cout << MB "File uploaded!" << std::endl;
	outputFile.write(&formData.data[pos + 4], dataLen - (pos + 4) - boundary.size()); // +2 if end request (--)
	outputFile.close();
	handleGetRequest(connection, request);
	send(connection, "File uploaded! ", strlen("File uploaded! "), 0);
	return "200";
}

std::string handleUrlEncoded(int connection, s_request request) {
	std::cout << DV "handleUrlEncoded" << std::endl;
	
	char buffer[1000];
	size_t bytes = 0;
	std::string data;

	// read on socket
	while ((bytes = recv(connection, buffer, 999, 0)) > 0 && bytes != std::string::npos)
	{
		data += buffer;
		if (stoul(request.headers["Content-Length"]) == data.length())
			break;
	}

	// parse data
	std::map<std::string, std::string> parsedData;
	std::istringstream iss(data);
	std::string line;
	while (std::getline(iss, line, '&'))
	{
		std::istringstream lineStream(line);
		std::string key;
		std::string value;
		std::getline(lineStream, key, '=');
		std::getline(lineStream, value);
		parsedData[key] = value;
	}

	handleGetRequest(connection, request);

	// write data in cout
	std::map<std::string, std::string>::const_iterator it;
	for (it = parsedData.begin(); it != parsedData.end(); ++it)
	{
		std::cout << MB << it->first << C ": " DV << it->second << std::endl;
		send(connection, it->first.c_str(), it->first.length(), 0);
		send(connection, ": ", 2, 0);
		send(connection, it->second.c_str(), it->second.length(), 0);
		send(connection, " ", 1, 0);
	}
	return "200";
}

std::string handleJson(int connection, s_request request) {
	std::cout << DV "handleJson" << std::endl;
	(void)connection;
	(void)request;
	return "200";
}

void handlePostRequest(int connection, s_request request) {
	std::string status = "505";

	if (request.headers["Content-Type"].find("multipart/form-data") != std::string::npos)
		status = handleFormData(connection, request);
	else if (request.headers["Content-Type"].find("application/x-www-form-urlencoded") != std::string::npos)
		status = handleUrlEncoded(connection, request);
	else if (request.headers["Content-Type"].find("application/json") != std::string::npos)
		status = handleJson(connection, request);

	std::string response = "HTTP/1.1 " + status + " OK\r\n"; // TODO : map status code and message
	send(connection, response.c_str(), response.length(), 0);

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
