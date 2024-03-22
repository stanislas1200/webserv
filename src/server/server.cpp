#include "../../include/Webserv.hpp"

// inet_ntoa
#include <arpa/inet.h>

std::string readHeader(int connection) {

	std::cout << "Read header" << std::endl;

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

int handleConnection(s_request *request) {

	std::cout << "Handle connection" << std::endl;

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
	int flag = fcntl(server->fd, F_GETFL, 0);
	if (fcntl(server->fd, F_SETFL, flag | O_NONBLOCK) < 0)
		return close(server->fd), error("Sock opt:", strerror(errno), NULL), -1;

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
		{
			// // new connection
			// struct sockaddr_in client_addr;
			// socklen_t client_len = sizeof(client_addr);
			// int client_fd = accept(servers[j].fd, (struct sockaddr*)&client_addr, &client_len);
			// std::cout << "Accept: " << client_fd << std::endl;
			// if (client_fd == -1)
			// 	return error("Accept:", strerror(errno), NULL), -1; // TODO : close all fd
			
			// // set non blocking
			// int flag = fcntl(client_fd, F_GETFL, 0);
			// if (fcntl(client_fd, F_SETFL, flag | O_NONBLOCK) < 0)
			// 	return close(client_fd), error("Sock opt:", strerror(errno), NULL), -1; // TODO : close all fd
			
			// // add client to epoll
			// struct epoll_event client_event;
			// client_event.events = EPOLLIN | EPOLLOUT | EPOLLET; // edge triggered mode
			// client_event.data.fd = client_fd;
			// if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1)
			// 	return close(client_fd), error("Epoll:", strerror(errno), NULL), -1; // TODO : close all fd
			// std::cout << GREEN "New connection: " DV << client_fd << " " << inet_ntoa(client_addr.sin_addr) << " " << ntohs(client_addr.sin_port) << C << std::endl;
			return 1;
		}
	}
	return 0;
}

void serverRun(std::vector<s_server> servers, int max_fd, size_t fd_size) {

	// server using select 
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
					// check if it.connection is in vectior severs using vectior buildin
					// if (std::find(servers.begin(), servers.end(), it->second) != servers.end())
					// 	continue;
					// if (!isServerConnection(servers, it->second.connection))
					// {
					// 	std::cout << YELLOW "server" << std::endl;
					// 	continue;
					// }
					std::cout << RED << "OK1" << std::endl;
					if (handleConnection(&it->second))
					{
						std::cout << GREEN << "close connection" << std::endl;
						// close(request.connection);
						// if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, it->second.connection, NULL) == -1)
						// 	return close(epoll_fd), error("Epoll M:", strerror(errno), NULL); // TODO : close all fd
						close(it->second.connection);
						std::cout << "Erase1 " << request_map.size() << std::endl;
						request_map.erase(it->second.connection);
						std::cout << "Erase1 " << request_map.size() << std::endl;
						break;
					}
				}
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
					std::cout << RED "\nFind a connection: " << servers[i].fd << std::endl;
					int addrlen = sizeof(servers[i].sockaddr);
					// if (servers[i].requests.size() == 0) 
					// {
						// new connection
						s_request	request;
						std::cout << "New connection" << std::endl;
						request.connection = accept(servers[i].fd, (struct sockaddr*)&servers[i].sockaddr, (socklen_t*)&addrlen);
						std::cout << "Accept: " << request.connection << std::endl;
						if (request.connection < 0)
						{
							error("Accept:", strerror(errno), NULL);
						}
						// else
						// 	servers[i].requests.push_back(request);
					
					// std::cout << "Request: " << servers[i].requests[0].path << std::endl;
					
					// handle request
					int client_fd = request.connection;
					// s_request request;
					// if (request_map.find(client_fd) == request_map.end())
					// {
					// request.connection = client_fd;
					request_map[request.connection] = request;
					std::cout << RED << "OK2" << std::endl;

					if (handleConnection(&request)) // read using recv return 1 if all is read else 0 if only buffer_size is read
					{
						std::cout << GREEN << "close connection" << std::endl;
						close(request.connection);
						std::cout << "Erase2 " << request_map.size() << std::endl;
						request_map.erase(request.connection);
						std::cout << "Erase2 " << request_map.size() << std::endl;
					}
					else
					{
						std::cout << "Add request" << std::endl;
						request_map[client_fd] = request;
					}
				
				}
			}
		}
		else
			error("Select:", strerror(errno), NULL);		

	}

	return;
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
