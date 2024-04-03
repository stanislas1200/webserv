#include "../../include/Webserv.hpp"

// inet_ntoa
#include <arpa/inet.h>

int handleConnection(s_request *request) {
	std::string header;
	if (request->headers.size() == 0)
		header = readHeader(request->connection);
	return (parseRequest(header, request));
}

int serverSetup(s_server *server) {
	server->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server->fd == -1)
		return error("Socket:", strerror(errno), NULL), -1;

	// non blocking // The connection was reset RIP
	// int flag = fcntl(server->fd, F_GETFL, 0);
	// if (fcntl(server->fd, F_SETFL, flag | O_NONBLOCK) < 0)
	// 	return close(server->fd), error("Sock opt:", strerror(errno), NULL), -1;

	// reusable sd
	int on = 1;
	if (setsockopt(server->fd, SOL_SOCKET, SO_REUSEADDR, (char *)&(on), sizeof(on)) < 0)
		return close(server->fd), error("Sock opt:", strerror(errno), NULL), -1;

	server->sockaddr.sin_family = AF_INET;
	server->sockaddr.sin_addr.s_addr = INADDR_ANY;
	server->sockaddr.sin_port = htons(server->port);
	
	if (bind(server->fd, (struct sockaddr*)&server->sockaddr, sizeof(server->sockaddr)) == -1)
		return close(server->fd), error("Bind:", strerror(errno), NULL), -1;

	if (listen(server->fd, 10) == -1)
		return close(server->fd), error("Listen:", strerror(errno), NULL), -1;

	return 0;
}

int isServerConnection(std::vector<s_server> servers, int fd) {
	for (size_t j = 0; j < servers.size(); j++)
	{
		if (fd == servers[j].fd)
			return 1;
	}
	return 0;
}

void serverRun(std::vector<s_server> servers, int max_fd, size_t fd_size) {
	std::cout << C"[" DV "serverRun" C "] " << YELLOW "---START---" C << std::endl;
	std::vector<s_request> requests;
	std::map<int, s_request> request_map;
	
	while (true)
	{
		fd_set reading_set;
		// fd_set writing_set;
		struct timeval tv;
		int ret = 0;

		while (!ret)
		{
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			// ft_memcpy(&reading_set, &_fd_set, sizeof(_fd_set));
			// FD_ZERO(&writing_set);
			

			for (size_t i = 0; i < fd_size; i++)
			{
				FD_SET(servers[i].fd, &reading_set);
			}

			if (request_map.size() > 0) // handle chuncked data
			{
				for (std::map<int, s_request>::iterator it = request_map.begin(); it != request_map.end(); ++it)
				{
					if (handleConnection(&it->second))
					{
						std::cout << C"[" DV "serverRun" C "] " << MB "close chunck connection" C << std::endl;
						close(it->second.connection);
						request_map.erase(it->second.connection);
						break;
					}
				}
			}

			std::cout << C"\r[" DV "serverRun" C "] " << GREEN "waiting a connection" C << std::flush;
			ret = select(max_fd + 1, &reading_set, NULL, NULL, &tv); // get the number of ready file descriptors
		}

		std::cout << "\n" << std::endl;
		if (ret > 0)
		{
			for (size_t i = 0; i < fd_size; i++)
			{
				if (FD_ISSET(servers[i].fd, &reading_set)) // check if the fd is in set
				{
					// new connection
					int addrlen = sizeof(servers[i].sockaddr);
					s_request	request;
					request.connection = accept(servers[i].fd, (struct sockaddr*)&servers[i].sockaddr, (socklen_t*)&addrlen);
					std::cout << C"[" DV "serverRun" C "] " << MB "connection fd" C ": " GREEN << request.connection << std::endl;
					if (request.connection < 0)
					{
						error("Accept:", strerror(errno), NULL);
					}

					// add connection to map
					request_map[request.connection] = request;

					// handle connection
					if (handleConnection(&request)) // read using recv return 1 if all is read else 0 if only buffer_size is read
					{
						std::cout << C"[" DV "serverRun" C "] " << MB "close connection" C << std::endl;
						close(request.connection);
						request_map.erase(request.connection);
					}
					// update map
					request_map[request.connection] = request;
				}
			}
		}
		else
			error("Select:", strerror(errno), NULL);		

	}
	return;
}

void acceptConnection(std::vector<ServConfig> config) {
	(void)config;
    std::vector<s_server> servers;
	for (unsigned long i = 0; i < config.size(); i++)
	{
		s_server server;
		server.port = config[i].getPort();
		servers.push_back(server);
	}

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
