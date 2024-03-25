#include "../../include/request.hpp"

std::string readHeader(int connection) {
	std::string header;
	char buffer[1024];
	int bytes = 0;

	while ((bytes = recv(connection, buffer, 1, 0)) > 0) {
		buffer[bytes] = '\0';
		header += buffer;
		if (header.find("\r\n\r\n") != std::string::npos)
			break;
	}
	return header;
}

bool stringEnd(std::string file, std::string end) {
	if (file.size() < end.size())
		return false;

	int j = end.size();
	for (unsigned long i = file.size(); i > file.size() - end.size(); i--)
	{
		if (file[i] != end[j])
			return false;
		j--;
	}
	return true;
}

void	sendFile(int connection, std::ifstream *file, std::string status, std::string fileName) {

	std::string responce;
	std::stringstream ss;
	if (stringEnd(fileName, ".html"))
	{
		std::ifstream templat("src/pages/template.html");
		ss << templat.rdbuf();
		responce = ss.str();
		
		// Get response content
		ss.str(""); // emtpy
		ss << file->rdbuf();
		std::string fileContent = ss.str();

		size_t pos = responce.find("{{BODY}}");
		if (pos != std::string::npos) {
			if (file->is_open())
				responce.replace(pos, pos + 8, fileContent);
			else
				responce.replace(pos, pos + 8, "<h1 style=\"text-align:center\">Error 404 Not Found Error</h1>");
		}
		else
		{
			status = "500";
			responce = "<h1 style=\"text-align:center\">Error 500 Internal Server Error</h1>";
			error("File:", "Error in template file", NULL);
		}
		templat.close();
	}
	else
	{
		ss << file->rdbuf();
		responce = ss.str();
	}
	// Send status line
	std::string statusLine = "HTTP/1.1 " + status + " OK\r\n"; // TODO : map status code and message
	send(connection, statusLine.c_str(), statusLine.length(), 0);
    send(connection, "Content-Type: text/html\r\n\r\n", strlen("Content-Type: text/html\r\n\r\n"), 0);

	send(connection, responce.c_str(), responce.size(), 0);

	file->close();
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
	// Parse request header if needed
	int connection = request->connection;
	if (request->method.empty())
	{
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
	// handle methode
	std::cout << C"[" DV "parseRequest" C "] " << MB "METHOD" C ": " GREEN << request->method << C << std::endl;
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
