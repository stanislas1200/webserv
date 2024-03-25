#pragma once

# include <iostream>
# include <string>
# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <errno.h>
# include <string.h>
# include <fstream>
# include <sstream>
# include <stdlib.h>
# include <map>
# include <sstream>
# include <iomanip>
# include <vector>
# include <iostream>
# include <fcntl.h>
# include "ServConfig.hpp"

# define RED "\x1b[1m\x1b[38;2;255;0;0m"
# define MB "\x1b[1m\x1b[38;2;25;25;599m"
# define GREEN "\x1b[1m\x1b[38;2;0;128;0m"
# define YELLOW "\x1b[1m\x1b[38;2;205;205;0m"
# define DV "\x1b[1m\x1b[38;2;148;0;211m"
# define C "\x1b[0m"

void error(const char *type, const char *msg, const char *bold);

typedef struct s_file {
	std::string name;
	std::string	path;
}	t_file;

typedef struct s_config {
	int port;
}	t_config;

typedef struct s_FormDataPart {
    std::string header;
	std::string name;
	std::string filename;
	std::string contentType;
	std::vector<char> data;
	int dataLen = 0;
} t_FormDataPart;

typedef struct s_request {
	std::string method;
	std::string path;
	std::map<std::string, std::string> headers;
	std::string body;
	struct s_FormDataPart formData;
	int connection;
} t_request;

typedef struct s_server {
	int port;
	int fd;
	sockaddr_in sockaddr;
	std::vector<s_request> requests;
} t_server;

int parseRequest(std::string header, s_request *request);
void error(const char *type, const char *msg, const char *bold);
void acceptConnection(s_config config);
void sendFile(int connection, std::ifstream *file, std::string status);
void printRequest(s_request request);
