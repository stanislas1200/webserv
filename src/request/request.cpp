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

void handlePostRequest(int connection, s_request request) {
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
		std::cout << DV "dataHeader: " << dataHeader << std::endl;
		std::string data;
		std::vector<char> binary;
		size_t pos;
		pos = request.headers["Content-Type"].find("boundary=") + 9;
		std::string boundary = "--" + request.headers["Content-Type"].substr(pos, pos + 24);
		bool flag = 1;
		std::cout << atoi(request.headers["Content-Length"].c_str()) - sizeof(request) << std::endl;
		while (flag && (bytes = recv(connection, buffer, 1000, 0)) > 0)
		{
			std::cout << MB << bytes << std::endl;
			data += buffer;
			binary.insert(binary.end(), buffer, buffer + bytes);
			if ((pos = data.find(boundary)) != std::string::npos)
			{
				std::cout << RED << "BREAK" << std::endl;
				// binary.erase(binary.begin() + pos, binary.end());
				break;
			}			
		}
		std::cout << RED << "OUT" << bytes << std::endl;
		std::ofstream outputFile("output.png", std::ios::binary);
		if (!outputFile.is_open())
				error("Open:", strerror(errno), NULL);
		outputFile.write(&binary[0], binary.size());
		outputFile.close();
	}
	send(connection, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"), 0);
	return;
}

void handleDeleteRequest(int connection, s_request request) {
	// remove(request.)
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
	else if (request.method == "DELETE")
		handleDeleteRequest(connection, request);
	else
	{
		std::string response = "HTTP/1.0 505 Not supported\r\nContent-type:text/html\r\n\r\n";
		send(connection, response.c_str(), response.length(), 0);
	}
}
