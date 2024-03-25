#include "../../include/request.hpp"

int handleDeleteRequest(int connection, s_request request) { // TODO : replace space in filename
	if (std::remove(("upload/" + request.path).c_str()))
		error("DELETE:", "no file:", ("upload/" + request.path).c_str());
	send(connection, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"), 0);
	return 1;
}