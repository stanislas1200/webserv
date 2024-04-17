#include "../../include/request.hpp"
std::string	runCgi(s_request& request);


int handleGetRequest(int connection, s_request request) {
	std::string status = "200";

	// CGI
	std::string path = request.loc.getRedirection();
	std::string fileContent;
	if (path.substr(0, request.loc.getPathToCgi().size()) == request.loc.getPathToCgi() && request.loc.getExCgi().find(path.substr(path.find_last_of('.') + 1)) != std::string::npos) {
		std::cout << C"[" GREEN "handleGetRequest" C "] " << YELLOW "CGI" C << std::endl;
		try {
			request.path = path;
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

	std::ifstream file(path.c_str(), std::ios::binary);

	if (!file.is_open()) {
		error("File:", strerror(errno), path.c_str());
		status = "404"; // sendfile handle error; make a class ?
		path = request.conf.getErrorPages()[404];
		file.open(path, std::ios::binary);
	}
	
	sendFile(connection, &file, status, path, request);
	return 1;
}