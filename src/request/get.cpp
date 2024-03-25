#include "../../include/request.hpp"

int handleGetRequest(int connection, s_request request) {
	
	std::string path = request.path;
	// TODO : CGI
	if (path == "/") {
		path = "/index.html";
	}

	std::string fullPath = "src/pages" + path; // src/pages/../main.cpp
	std::ifstream file(fullPath.c_str(), std::ios::binary);
	std::string status = "200";

	if (!file.is_open()) {
		status = "404"; // sendfile handle error; make a class ?
		file.open("src/pages/errorpages/error_404.html", std::ios::binary);
		path = "error_404.html";
		error("File:", strerror(errno), fullPath.c_str());
	}
	
	sendFile(connection, &file, status, path);
	return 1;
}