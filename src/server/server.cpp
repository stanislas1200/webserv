#include "../../include/Webserv.hpp"

// inet_ntoa
#include <arpa/inet.h>

int handleConnection(s_request *request) {
	std::string header;
	if (request->headers.size() == 0)
		header = readHeader(request->connection);
	return (parseRequest(header, request));
}

int serverSetup(ServConfig *server) {
	server->setFd(socket(AF_INET, SOCK_STREAM, 0));
	if (server->getFd() == -1)
		return error("Socket:", strerror(errno), NULL), -1;

	// non blocking // The connection was reset RIP
	int flag = fcntl(server->getFd(), F_GETFL, 0);
	if (fcntl(server->getFd(), F_SETFL, flag | O_NONBLOCK) < 0)
		return close(server->getFd()), error("Sock opt:", strerror(errno), NULL), -1;

	// reusable sd
	int on = 1;
	if (setsockopt(server->getFd(), SOL_SOCKET, SO_REUSEADDR, (char *)&(on), sizeof(on)) < 0)
		return close(server->getFd()), error("Sock opt:", strerror(errno), NULL), -1;

	// sockaddr_in addr;

	server->_sockaddr.sin_family = AF_INET;
	server->_sockaddr.sin_addr.s_addr = INADDR_ANY;
	server->_sockaddr.sin_port = htons(server->getPort());

	// server->setSockaddr(addr);
	
	if (bind(server->getFd(), (struct sockaddr*)&server->_sockaddr, sizeof(server->_sockaddr)) == -1)
		return close(server->getFd()), error("Bind:", strerror(errno), NULL), -1;

	if (listen(server->getFd(), 10) == -1) // TODO : check max 
		return close(server->getFd()), error("Listen:", strerror(errno), NULL), -1;

	return 0;
}

# include <poll.h>
void serverRun(std::vector<ServConfig> servers, int max_fd, size_t fd_size) {
	std::cout << C"[" DV "serverRun" C "] " << YELLOW "---START---" C << std::endl;
	std::vector<s_request> requests;
	std::map<int, s_request> request_map;

	std::vector<pollfd> fds;
	pollfd				fill;

	for (size_t i = 0; i < fd_size; i++) {
		fill.fd = servers[i].getFd();
		fill.events = POLLIN;
		fill.revents = 0;
		fds.push_back(fill);
	}
	while (true)
	{
		(void)max_fd;
		int ret = 0;

		while (!ret)
		{

			std::cout << C"\r[" DV "serverRun" C "] " << GREEN "waiting a connection, in queue : " C << request_map.size() << std::flush;
			ret = poll(fds.data(), fds.size(), -1);
			if (ret == -1)
			{
				std::cerr << "Error in poll(): " << strerror(errno) << std::endl;
				continue;
			}
			if (request_map.size() > 0) // handle client connection
			{
				int j = 0;
				for (std::map<int, s_request>::iterator it = request_map.begin(); it != request_map.end(); ++it)
				{
					if (fds[fd_size + j].revents & POLLIN || it->second.formData[0].full)
					{
						if (handleConnection(&it->second))
						{
							std::cout << C"[" DV "serverRun" C "] " << MB "close chunck connection" C << std::endl;
							close(it->second.connection);
							request_map.erase(it->second.connection);
							fds.erase(fds.end() - request_map.size() - 1 + j);
							break;
						}
					}
					j++;
				}
			}
		}

		if (ret > 0)
		{
			for (size_t i = 0; i < fd_size; i++)
			{
				if (fds[i].revents & POLLIN) // check if the fd is ready
				{
					// new connection
					int addrlen = sizeof(servers[i]._sockaddr);
					s_request	request;
					request.dataLen = 0;
					request.formData[0].full = false;
					request.formData[1].full = false;

					request.connection = accept(servers[i].getFd(), (struct sockaddr*)&servers[i]._sockaddr, (socklen_t*)&addrlen);
					request.conf = servers[i];
					// non blocking // The connection was reset RIP
					int flag = fcntl(request.connection, F_GETFL, 0);
					if (fcntl(request.connection, F_SETFL, flag | O_NONBLOCK) < 0)
						return close(request.connection), error("Sock opt:", strerror(errno), NULL);
					
					if (request.connection < 0)
					{
						error("Accept2:", strerror(errno), NULL);
						break;
					}
					int on = 1;
					if (setsockopt(request.connection, SOL_SOCKET, SO_REUSEADDR, (char *)&(on), sizeof(on)) < 0)
					{
						close(request.connection), error("Sock opt:", strerror(errno), NULL);
						break;
					}
					if (request.connection != -1)
					{
						fill.fd = request.connection;
						fill.events = POLLIN;
						fill.revents = 0;
						fds.push_back(fill);
						request_map[request.connection] = request;
						std::cout << C"[" DV "serverRun" C "] " << MB "connection fd" C ": " GREEN << request.connection << std::endl;
					}
					
				}
			}
		}
		else
			error("Select:", strerror(errno), NULL);		

	}
	return;
}

void acceptConnection(std::vector<ServConfig> servers) {
	int max_fd;
	size_t fd_size;
	fd_size = servers.size();
	max_fd = 0;

	for (size_t i = 0; i < fd_size; i++)
	{
		std::cout << GREEN "server: " MB << i << std::endl;
		if (serverSetup(&servers[i]) != -1) // socket bind listen
		{
			if (servers[i].getFd() > max_fd) 
				max_fd = servers[i].getFd();
			std::cout << YELLOW "server port: " MB << servers[i].getPort() << std::endl;
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
