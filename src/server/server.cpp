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

int isServerConnection(std::vector<s_server> servers, int fd, int epoll_fd) {
	for (size_t j = 0; j < servers.size(); j++)
	{
		if (fd == servers[j].fd)
		{
			// new connection
			struct sockaddr_in client_addr;
			socklen_t client_len = sizeof(client_addr);
			int client_fd = accept(servers[j].fd, (struct sockaddr*)&client_addr, &client_len);
			std::cout << "Accept: " << client_fd << std::endl;
			if (client_fd == -1)
				return error("Accept:", strerror(errno), NULL), -1; // TODO : close all fd
			
			// set non blocking
			int flag = fcntl(client_fd, F_GETFL, 0);
			if (fcntl(client_fd, F_SETFL, flag | O_NONBLOCK) < 0)
				return close(client_fd), error("Sock opt:", strerror(errno), NULL), -1; // TODO : close all fd
			
			// add client to epoll
			struct epoll_event client_event;
			client_event.events = EPOLLIN | EPOLLOUT | EPOLLET; // edge triggered mode
			client_event.data.fd = client_fd;
			if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1)
				return close(client_fd), error("Epoll:", strerror(errno), NULL), -1; // TODO : close all fd
			std::cout << GREEN "New connection: " DV << client_fd << " " << inet_ntoa(client_addr.sin_addr) << " " << ntohs(client_addr.sin_port) << C << std::endl;
			return 1;
		}
	}
	return 0;
}

void serverRun(std::vector<s_server> servers, int max_fd, size_t fd_size) {

	// store requests
	std::vector<s_request> requests;
	std::map<int, s_request> request_map;

	// epoll instance
	int epoll_fd = epoll_create(fd_size);
	if (epoll_fd == -1)
		return error("Epoll:", strerror(errno), NULL); // TODO : close all fd

	// add servers to epoll
	for (size_t i = 0; i < fd_size; i++)
	{
		struct epoll_event event;
		event.events = EPOLLIN;
		event.data.fd = servers[i].fd;
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, servers[i].fd, &event) == -1)
			return close(epoll_fd), error("Epoll:", strerror(errno), NULL); // TODO : close all fd
	}
	int ret = 0;
	while (true)
	{
		struct epoll_event events[fd_size];
		
		std::cout << GREEN "\rWait " << max_fd << std::flush;
		while (request_map.size() > 0)
		{
			for (std::map<int, s_request>::iterator it = request_map.begin(); it != request_map.end(); ++it)
			{
				std::cout << RED << "OK" << std::endl;
				if (handleConnection(&it->second))
				{
					std::cout << GREEN << "close connection" << std::endl;
					// close(request.connection);
					if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, it->second.connection, NULL) == -1)
						return close(epoll_fd), error("Epoll M:", strerror(errno), NULL); // TODO : close all fd
					close(it->second.connection);
					request_map.erase(it->second.connection);
					std::cout << "Erase" << std::endl;
				}
			}
		}




		// wait for events
		ret = epoll_wait(epoll_fd, events, fd_size, -1);
		if (ret == -1)
			return close(epoll_fd), error("Epoll:", strerror(errno), NULL); // TODO : close all fd
		
		// handle events
		for (int i = 0; i < ret; i++)
		{
			if (events[i].events & EPOLLIN)
			{
				int on = isServerConnection(servers, events[i].data.fd, epoll_fd); // TODO : handle faillure
				if (on == -1)
					return;
				if (on)
					continue;

				// handle request
				int client_fd = events[i].data.fd;
				s_request request;
				// if (request_map.find(client_fd) == request_map.end())
				// {
					request.connection = client_fd;
					request_map[client_fd] = request;
				// }
				// else
				// {
				// 	std::cout << RED << "Old request" << std::endl;
				// 	request = request_map[client_fd];
				// }
				// request.connection = events[i].data.fd;
				std::cout << MB "Request: " DV << request.connection << C << std::endl;
				// for (std::map<int, s_request>::iterator it = request_map.begin(); it != request_map.end(); ++it)
				// {
				// 	std::cout << MB << it->first << C << std::endl;
				// }
				if (handleConnection(&request)) // read using recv return 1 if all is read else 0 if only buffer_size is read
				{
					std::cout << GREEN << "close connection" << std::endl;
					// close(request.connection);
					if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, request.connection, NULL) == -1)
						return close(epoll_fd), error("Epoll R:", strerror(errno), NULL); // TODO : close all fd
					close(request.connection);
					request_map.erase(request.connection);
				}
				else
				{
					std::cout << "Add request" << std::endl;
					request_map[client_fd] = request;
				}
			}
		}
	}
	std::cout << "End" << std::endl;

	return;
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
			for (size_t i = 0; i < fd_size; i++)
			{
				if (servers[i].requests.size() > 0 && handleConnection(&servers[i].requests[0]))
				{
					std::cout << GREEN << "close connection" << std::endl;
					close(servers[i].requests[0].connection);
					servers[i].requests.erase(servers[i].requests.begin() + 0);
				}
			}
		}

		if (ret > 0)
		{
			for (size_t i = 0; i < fd_size; i++)
			{
				if (FD_ISSET(servers[i].fd, &reading_set)) // check if the fd is in set
				{
					std::cout << RED "\nFind a connection: " << servers[i].fd << std::endl;
					int addrlen = sizeof(servers[i].sockaddr);
					// non blocking // The connection was reset RIP // not working anymore
					// if (fcntl(servers[i].fd, F_SETFL, O_NONBLOCK) < 0)
					// 	error("Sock opt:", strerror(errno), NULL);

					
					if (servers[i].requests.size() == 0) // TODO : handle old connection 
					{
						// new connection
						s_request	request;
						std::cout << "New connection" << std::endl;
						request.connection = accept(servers[i].fd, (struct sockaddr*)&servers[i].sockaddr, (socklen_t*)&addrlen);
						std::cout << "Accept: " << request.connection << std::endl;
						if (request.connection < 0)
						{
							error("Accept:", strerror(errno), NULL);
						}
						else
							servers[i].requests.push_back(request);
					}
					
					// if (connection < 0)
					// {
					// 	if (errno != EAGAIN)
					// 		error("Connection:", strerror(errno), NULL);
					// 	continue;
					// }
					std::cout << "Request: " << servers[i].requests[0].path << std::endl;
					if (handleConnection(&servers[i].requests[0]))
					{
						std::cout << GREEN << "close connection" << std::endl;
						close(servers[i].requests[0].connection);
						servers[i].requests.erase(servers[i].requests.begin() + 0);
					}
					std::cout << "continue" << std::endl;

					// std::cout << "New connection: " << servers[i].requests.size() << std::endl;
					// for (size_t j = 0; j < servers[i].requests.size(); j++)
					// {
					// 	std::cout << MB << "Handle connection: " << servers[i].requests[j].connection << std::endl;
					// 	if (handleConnection(&servers[i].requests[j]))
					// 	{
					// 		std::cout << GREEN << "close connection" << std::endl;
					// 		close(servers[i].requests[j].connection);
					// 		servers[i].requests.erase(servers[i].requests.begin() + j);
					// 	}
					// }

					//push server at end
					servers.push_back(servers[i]);
					servers.erase(servers.begin() + i);
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
