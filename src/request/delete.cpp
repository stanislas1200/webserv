#include "../../include/request.hpp"

int handleDeleteRequest(int connection, s_request request) { // TODO : replace space in filename
	if (request.path == "/") {
		send(connection, "HTTP/1.1 400 Bad Request\r\n", strlen("HTTP/1.1 400 Bad Request\r\n"), 0);
		return error("DELETE:", "no file specified", NULL), 1;
	}
	if (std::remove(("upload/" + request.path).c_str()))
	{
		send(connection, "HTTP/1.1 404 Not Found\r\n", strlen("HTTP/1.1 404 Not Found\r\n"), 0);
		return error("DELETE:", "no file:", ("upload/" + request.path).c_str()), 1;
	}
	send(connection, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"), 0);
	return 1;
}