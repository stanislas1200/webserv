#include "../include/Webserv.hpp"

void error(const char *type, const char *msg, const char *bold) {
	std::cerr << RED "Error: " YELLOW << type << " " C << msg;
	if (bold)
		std::cerr << DV " " << bold << C;
	std::cerr << C << std::endl;
	exit(1);
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
}

std::string readHeader(int connection) {
	std::string header;
	char buffer[1024];
	int bytes = 0;

	while ((bytes = recv(connection, buffer, 1, 0)) > 0) {
		buffer[bytes] = '\0';
		std::cout << buffer << std::endl;
		header += buffer;
		if (header.find("\r\n\r\n") != std::string::npos)
			break;
	}
	return header;
}


void handleConnection(int connection) {
	
	std::string request = readHeader(connection);
	parseRequest(connection, request);
}

int main(int ac, char **av) {
	s_config config;

	if (ac != 2) {
		// error("Usage:", av[0], "<config_file>");
		std::cout << GREEN "WebServ: " << MB "Using default config file" C << std::endl;
		getConfig(&config, "configs/default");
	}
	else
		getConfig(&config, av[1]);

	int socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFd == -1)
		error("Socket:", strerror(errno), NULL);
	
	// reusable sd
	int on = 1;
	int rc = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, (char *)&(on), sizeof(on));

	//

	if (rc < 0)
		error("Sock opt:", strerror(errno), NULL);

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
			
		// non blocking
		// rc = fcntl(connection, F_SETFL, O_NONBLOCK);
		// if (rc < 0)
		// 	error("Sock opt:", strerror(errno), NULL);
		
		handleConnection(connection);

		close(connection);
	}
}