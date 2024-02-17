#include "../include/Webserv.hpp"

void error(const char *type, const char *msg, const char *bold) {
	std::cerr << RED "Error: " YELLOW << type << " " C << msg;
	if (bold)
		std::cerr << DV " " << bold << C;
	std::cerr << C << std::endl;
}

void getConfig(s_config *config, std::string file) {
	std::ifstream confFILE(file.c_str());
	if (!confFILE.is_open()) {
		error("Config:", strerror(errno), NULL);
		return ;
	}

	std::string line;
	while (std::getline(confFILE, line)) {
		std::cout << line << std::endl;
		if (line.find("listen") != std::string::npos) {
			config->port = atoi(line.substr(line.find(" ") + 1).c_str());
			if (config->port < 0 || config->port > 65535)
				error("Port:", "Invalid port number", NULL);
		}
	}
    confFILE.close();

	std::ifstream confFile(file.c_str());
    ServConfig  configClass;
    try {
        configClass.initializeConfig(&confFile);
    }
    catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}

void	sendFile(int connection, std::ifstream *file) {
	char buffer[1024];

	send(connection, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"), 0);
    send(connection, "Content-Type: text/html\r\n\r\n", strlen("Content-Type: text/html\r\n\r\n"), 0);
    while (file->read(buffer, sizeof(buffer)).gcount() > 0) {
		std::cout << "Sending file" << std::endl;
		std::cout << file->gcount() << std::endl;
        send(connection, buffer, file->gcount(), 0);
    }
}

void handleGetRequest(int connection, std::string request) {
	char filePath[1024];
	sscanf(request.c_str(), "GET %255s", filePath);
	std::string path = filePath;
	if (path == "/") {
		path = "/index.html";
	}
	std::string fullPath = "src/pages" + path;
	std::cout << "Full path: " << fullPath << std::endl;
	std::ifstream file(fullPath.c_str(), std::ios::binary);
	if (!file.is_open()) {
		std::ifstream fileError("src/pages/errorpages/error_404.html", std::ios::binary);
		sendFile(connection, &fileError);
		error("File:", strerror(errno), NULL);
		return ;

		// std::string response = "HTTP/1.0 404 Not Found\r\nContent-type:text/html\r\n\r\n";
		// std::string page = "<html>\n<head>\n<title>404 Not Found</title>\n</head>\n<body>\n<h2>404 Not Found</h2>\n</body>\n</html>\n\n";
		// send(connection, response.c_str(), response.length(), 0);
		// send(connection, page.c_str(), page.length(), 0);
		// return ;
	}
    sendFile(connection, &file);

  // Send file content
   
    file.close();
}

void handlePostRequest(int connection, std::string request) {
	std::cout << connection << "  " MB << request << std::endl;
	// Find the boundary string
    std::string boundaryMarker = "boundary=";
    size_t boundaryPos = request.find(boundaryMarker);
    // if (boundaryPos == std::string::npos) {
    //     error("Boundary not found in request");
    // }

    boundaryPos += boundaryMarker.length();
    size_t endOfLine = request.find("\r\n", boundaryPos);
    // if (endOfLine == std::string::npos) {
    //     error("Failed to find end of boundary line");
    // }

    std::string boundary = request.substr(boundaryPos, endOfLine - boundaryPos);

	send(connection, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"), 0);
	// // std::string data = request.substr(request.find("\r\n\r\n"), request.back());
	std::string data = request.substr(endOfLine);
	// std::string boundary = data.substr(9, data.back());
	// std::cout << GREEN "DATA:\n" << boundary << std::endl;
	// boundary = boundary.substr(0, boundary.find("\n"));
	// // data = data.substr(request.find("\n"), request.back());
	std::cout << GREEN "DATA:\n" << data << std::endl;

	std::string fileData = data.substr(data.find(boundary));
	std::cout << YELLOW "DATA:\n" << fileData << std::endl; 
	std::ofstream outputFile("output", std::ios::binary);
	if (!outputFile.is_open())
		error("Open:", strerror(errno), NULL);
	else
		outputFile.write(fileData.c_str(), fileData.length());
	outputFile.close();
	// const char* dataStart = strstr(request, "\r\n\r\n");
	// if (!dataStart)
	// 	error("HTTP", "Invalid POST request", NULL);
	// std::cout << dataStart << std::endl;
}

void handleDeleteRequest(int connection, std::string request) {
	(void)connection;
	(void)request;
}

void parseRequest(int connection, char *buffer) {
	std::string request(buffer);
	std::string method = request.substr(0, request.find(" "));

	std::cout << "Method: " << method << std::endl;
	if (method == "GET")
		handleGetRequest(connection, request);
	else if (method == "POST")
		handlePostRequest(connection, request);
	else if (method == "DELETE")
		handleDeleteRequest(connection, request);
	else
	{
		std::string response = "HTTP/1.0 505 Not supported\r\nContent-type:text/html\r\n\r\n";
		send(connection, response.c_str(), response.length(), 0);
	}
}

void handleConnection(int connection) {
	
		char buffer[10240];
		read(connection, buffer, sizeof(buffer) - 1);

		parseRequest(connection, buffer);

		// std::string response = "HTTP/1.0 200 OK\r\nContent-type:text/html\r\n\r\n";
		// std::string page = "<html>\n<head>\n<title>Hello World - First CGI Program</title>\n</head>\n<body>\n<h2>Hello World! This is my first CGI program</h2>\n</body>\n</html>\n\n";

		// send(connection, response.c_str(), response.length(), 0);
		// send(connection, page.c_str(), page.length(), 0);
		// std::cout << buffer << std::endl;
}

int main(int ac, char **av) {
	s_config config;

	if (ac != 2) {
		error("Usage:", av[0], "<config_file>");
		std::cout << GREEN "WebServ: " << MB "Using default config file" C << std::endl;
		getConfig(&config, "configs/default");
	}
	else
		getConfig(&config, av[1]);

	int socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFd == -1)
		error("Socket:", strerror(errno), NULL);
	

	sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(config.port);
	if (bind(socketFd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1)
		error("Bind:", strerror(errno), NULL);

	if (listen(socketFd, 10) == -1)
		error("Listen:", strerror(errno), NULL);

	while (1)
	{

		int addrlen = sizeof(sockaddr);
		int connection = accept(socketFd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
		if (connection < 0)
			error("Connection:", strerror(errno), NULL);

		handleConnection(connection);

		close(connection);
	}
}