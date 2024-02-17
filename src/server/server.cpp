#include "../../include/Webserv.hpp"

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

void handleConnection(int connection) {
	std::string request = readHeader(connection);
	parseRequest(connection, request);
}

int serverSetup(s_server *server) {
	server->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server->fd == -1)
		return error("Socket:", strerror(errno), NULL), -1;

	// non blocking // The connection was reset RIP
	if (fcntl(server->fd, F_SETFL, O_NONBLOCK) < 0)
		error("Sock opt:", strerror(errno), NULL);

	// reusable sd
	int on = 1;
	if (setsockopt(server->fd, SOL_SOCKET, SO_REUSEADDR, (char *)&(on), sizeof(on)) < 0)
		error("Sock opt:", strerror(errno), NULL);

	server->sockaddr.sin_family = AF_INET;
	server->sockaddr.sin_addr.s_addr = INADDR_ANY;
	server->sockaddr.sin_port = htons(server->port);
	
	if (bind(server->fd, (struct sockaddr*)&server->sockaddr, sizeof(server->sockaddr)) == -1)
		return error("Bind:", strerror(errno), NULL), -1;

	if (listen(server->fd, 100) == -1)
		return error("Listen:", strerror(errno), NULL), -1;

	return 0;
}

void serverRun(std::vector<s_server> servers, int max_fd, size_t fd_size) {
	while (1)
	{
		fd_set reading_set;
		fd_set writing_set;
		struct timeval tv;
		int ret = 0;

		while (!ret)
		{
			// copy read from set ?
			FD_ZERO(&reading_set); 
			FD_ZERO(&writing_set);
			tv.tv_sec = 1;
			tv.tv_usec = 0;

			for (size_t i = 0; i < fd_size; i++)
			{
				FD_SET(servers[i].fd, &reading_set);
				// FD_SET(servers[i].fd, &writing_set);
			}

			std::cout << GREEN "\rWait " << max_fd << std::flush;
			ret = select(max_fd + 1, &reading_set, NULL, NULL, &tv); // get the number of ready file descriptors
		}

		if (ret > 0)
		{
			for (size_t i = 0; i < fd_size; i++)
			{
				if (FD_ISSET(servers[i].fd, &reading_set)) // check if the fd is in set
				{
					// std::cout << RED "\nFind a connection: " << servers[i].fd << std::endl;
					int addrlen = sizeof(servers[i].sockaddr);
					// non blocking // The connection was reset RIP // not working anymore
					if (fcntl(servers[i].fd, F_SETFL, O_NONBLOCK) < 0)
						error("Sock opt:", strerror(errno), NULL);
					int connection = accept(servers[i].fd, (struct sockaddr*)&servers[i].sockaddr, (socklen_t*)&addrlen); // accept the first connection
					if (connection < 0)
					{
						if (errno != EAGAIN)
							error("Connection:", strerror(errno), NULL);
						continue;
					}
					handleConnection(connection);
					close(connection);
				}
			}
		}
		else
			error("Select:", strerror(errno), NULL);		
	}
}

void acceptConnection(s_config config) {
	(void)config;
    std::vector<s_server> servers;
	s_server server1;
	s_server server2;
	server1.port = 8090;
	server2.port = 8091;
	servers.push_back(server1);
	servers.push_back(server2);

	int max_fd;
	size_t fd_size;
	fd_size = servers.size();
	max_fd = 0;

	for (size_t i = 0; i < fd_size; i++)
	{
		std::cout << GREEN "server: " MB << i << std::endl;
		if (serverSetup(&servers[i]) != -1) // socket bind listen
		{
			if (servers[i].fd > max_fd) 
				max_fd = servers[i].fd;
			std::cout << YELLOW "server port: " MB << servers[i].port << std::endl;
		}
		else
			error("Server setup:", "Failed to setup server", NULL);
	}

	if (max_fd == 0)
		error("Server setup:", "No server setup", NULL);
	else
		serverRun(servers, max_fd, fd_size);
	return;
}
