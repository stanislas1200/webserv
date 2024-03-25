#pragma once

#include "Webserv.hpp"
#include <chrono>

int handlePostRequest(int connection, s_request *request);
int handleGetRequest(int connection, s_request request);
int handleDeleteRequest(int connection, s_request request);
