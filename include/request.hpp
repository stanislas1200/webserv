#pragma once
#include "Webserv.hpp"


typedef struct s_FormDataPart {
    std::string header;
	std::string name;
	std::string filename;
	std::string contentType;
	std::vector<char> data;
} t_FormDataPart;

void handlePostRequest(int connection, s_request request);
void handleGetRequest(int connection, s_request request);