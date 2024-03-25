#pragma once

#include "Webserv.hpp"
#include <chrono>


// typedef struct s_FormDataPart {
//     std::string header;
// 	std::string name;
// 	std::string filename;
// 	std::string contentType;
// 	std::vector<char> data;
// } t_FormDataPart;

int handlePostRequest(int connection, s_request *request);
int handleGetRequest(int connection, s_request request);