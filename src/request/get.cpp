#include "../../include/request.hpp"
std::string	runCgi(s_request& request);


int handleGetRequest(int connection, s_request request) {
	int status = 200;

	// CGI
	std::string path = request.loc.getRedirection();
	std::string fileContent;
	if (path.substr(0, request.loc.getPathToCgi().size()) == request.loc.getPathToCgi() && request.loc.getExCgi().find(path.substr(path.find_last_of('.') + 1)) != std::string::npos) {
		std::cout << C"[" GREEN "handleGetRequest" C "] " << YELLOW "CGI" C << std::endl;
		try {
			request.path = path;
			fileContent = runCgi(request);
		} catch (const std::exception& e) {
			std::cerr << RED "Error: " YELLOW << e.what() << C << std::endl;
			return sendError(500, request), 1;
		}
		std::string response = responseHeader(status) + fileContent;
		return send(connection, response.c_str(), response.size(), 0), 1;
	}

	// FILE

	std::ifstream file(path.c_str(), std::ios::binary);

	if (!file.is_open()) {
		return sendError(404, request), 1;
	}
	
	sendFile(connection, &file, status, request);
	return 1;
}