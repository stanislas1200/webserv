#include "../../include/request.hpp"
std::string	runCgi(s_request& request);


int handleGetRequest(int connection, s_request request) {
	std::string status = "200";

	// CGI
	std::string path = request.path;
	std::string fileContent;
	if (path.find("/cgi/") == 0) { // TODO : special path for cgi or extension set in config file?
		std::cout << C"[" GREEN "handleGetRequest" C "] " << YELLOW "CGI" C << std::endl;
		try {
			fileContent = runCgi(request);
		} catch (const std::exception& e) {
			fileContent = "<h1 style=\"text-align:center\">500 Internal Server Error</h1>";
			status = "500";
			std::cerr << RED "Error: " YELLOW << e.what() << C << std::endl;
		}
		std::string statusLine = "HTTP/1.1 " + status + " OK\r\n"; // TODO : map status code and message
		send(connection, statusLine.c_str(), statusLine.length(), 0);
		//send(connection, "Content-Type: text/html\r\n\r\n", strlen("Content-Type: text/html\r\n\r\n"), 0);
		std::cout << "fileContent: " << fileContent << std::endl;
		return send(connection, fileContent.c_str(), fileContent.size(), 0);
	}

	// FILE
	if (path == "/") {
		path = "/index.html";
	}

	std::string fullPath = "src/pages" + path; // src/pages/../main.cpp
	std::ifstream file(fullPath.c_str(), std::ios::binary);

	if (!file.is_open()) {
		status = "404"; // sendfile handle error; make a class ?
		file.open("src/pages/errorpages/error_404.html", std::ios::binary);
		path = "error_404.html";
		error("File:", strerror(errno), fullPath.c_str());
	}
	
	sendFile(connection, &file, status, path);
	return 1;
}