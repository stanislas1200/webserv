#include "../include/Webserv.hpp"

void error(std::string str) {
	std::cout << RED "Error: " YELLOW << str << std::endl; 
	exit(1);
}

int main(int ac, char **av) {
	int socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFd == -1)
		error("socket");
	

	sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(443); // conf and multiple 
	if (bind(socketFd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1)
		error("bind");

	if (listen(socketFd, 10) == -1)
		error("listen");

	while (1)
	{

		int addrlen = sizeof(sockaddr);
		int connection = accept(socketFd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
		if (connection < 0)
			error("connection");

		char buffer[10000];
		read(connection, buffer, 10000);

		std::string response = "HTTP/1.0 200 OK\r\nContent-type:text/html\r\n\r\n";
		std::string page = "<html>\n<head>\n<title>Hello World - First CGI Program</title>\n</head>\n<body>\n<h2>Hello World! This is my first CGI program</h2>\n</body>\n</html>\n\n";

		send(connection, response.c_str(), response.length(), 0);
		send(connection, page.c_str(), page.length(), 0);
		std::cout << buffer << std::endl;

		close(connection);
	}

	(void)av;
	(void)ac;
}