#pragma once

# include <iostream>
# include <string>
# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <errno.h>
# include <string.h>
# include <fstream>
# include <stdlib.h>
# include <map>
# include <sstream>
# include <iomanip>
# include <vector>
# include <iostream>
# include <fcntl.h>

# define RED "\x1b[1m\x1b[38;2;255;0;0m"
# define MB "\x1b[1m\x1b[38;2;25;25;599m"
# define GREEN "\x1b[1m\x1b[38;2;0;128;0m"
# define YELLOW "\x1b[1m\x1b[38;2;205;205;0m"
# define DV "\x1b[1m\x1b[38;2;148;0;211m"
# define C "\x1b[0m"

typedef struct s_config {
	int port;
} t_config;

typedef struct s_request {
	std::string method;
	std::string path;
	std::map<std::string, std::string> headers;
	std::string body;
} t_request;

typedef struct s_FormDataPart {
	std::string name;
	std::string filename;
	std::string contentType;
	std::vector<char> data;
} t_FormDataPart;

typedef struct s_server {
	int port;
	int fd;
	sockaddr_in sockaddr;
} t_server;

void parseRequest(int connection, std::string buffer);
void error(const char *type, const char *msg, const char *bold);
void acceptConnection(s_config config);