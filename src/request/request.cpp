#include "../../include/request.hpp"

int handleGetRequest(int connection, s_request request) {
	
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
	return 1;
}

int handleDeleteRequest(int connection, s_request request) { // TODO : replace space in filename
	if (std::remove(("upload/" + request.path).c_str()))
		error("DELETE:", "no file:", ("upload/" + request.path).c_str());
	send(connection, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"), 0);
	return 1;
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

int parseRequest(std::string header, s_request *request) {
	std::cout << "Check request" << std::endl;
	int connection = request->connection;
	if (request->method.empty())
	{
		std::cout << "Parsing request" << std::endl;
		std::istringstream requestStream(header);
		std::string line;

		// Parse the first line of the request
		std::getline(requestStream, line);
		std::istringstream firstLineStream(line);
		firstLineStream >> request->method >> request->path;

		// Parse the headers
		while (std::getline(requestStream, line) && line != "\r") {
			std::istringstream headerStream(line);
			std::string headerName;
			std::string headerValue;
			std::getline(headerStream, headerName, ':');
			std::getline(headerStream, headerValue);
			request->headers[headerName] = headerValue;
		}
	}

	// // Parse the body
	// std::stringstream ss;
	// ss << requestStream.rdbuf();
	// request->body = ss.str();

	std::cout << MB "METHOD: " GREEN << request->method << C << std::endl;
	
	if (request->method == "GET")
		return handleGetRequest(connection, *request);
	else if (request->method == "POST")
		return handlePostRequest(connection, request);
	else if (request->method == "DELETE")
		return handleDeleteRequest(connection, *request);
	else
	{
		std::string response = "HTTP/1.0 505 Not supported\r\nContent-type:text/html\r\n\r\n";
		send(connection, response.c_str(), response.length(), 0);
	}
	return 1;
}
