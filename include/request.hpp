#pragma once

#include "Webserv.hpp"
// #include <chrono>

int handlePostRequest(int connection, s_request *request);
int handleGetRequest(int connection, s_request request);
int handleDeleteRequest(int connection, s_request request);
void sendError(int status, s_request req);
std::string useTemplate(std::string content, s_request request);
std::string responseHeader(int status, s_request request);
