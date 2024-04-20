#include "../../include/request.hpp"
std::vector<unsigned char>	runCgi(s_request& request);
void requestCgi(s_request& request);


int handleGetRequest(int connection, s_request request) {
	std::string path = request.loc.getRedirection();

	// CGI
	if (path.substr(0, request.loc.getPathToCgi().size()) == request.loc.getPathToCgi() && request.loc.getExCgi().find(path.substr(path.find_last_of('.') + 1)) != std::string::npos) {
		std::cout << C"[" GREEN "handleGetRequest" C "] " << YELLOW "CGI" C << std::endl;
		request.path = path;
		return requestCgi(request), 1;
	}

	// FILE
	std::ifstream file(path.c_str(), std::ios::binary);

	if (!file.is_open()) {
		return sendError(404, request), 1;
	}
	
	sendFile(connection, &file, 200, request); // TODO : remove status
	return 1;
}