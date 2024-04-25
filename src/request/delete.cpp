#include "../../include/request.hpp"
// TODO : send function with check return of basic send;

int handleDeleteRequest(int connection, s_request request) {
	if (request.path == "/") {
		if (send(connection, "HTTP/1.1 400 Bad Request\r\n", strlen("HTTP/1.1 400 Bad Request\r\n"), 0) == -1)
			error("Send:", "don't care", NULL);
		return error("DELETE:", "no file specified", NULL), 1;
	}
	if (std::remove(("upload/" + request.path).c_str())) // TODO :config path ?
	{
		if (send(connection, "HTTP/1.1 404 Not Found\r\n", strlen("HTTP/1.1 404 Not Found\r\n"), 0) == -1)
			error("Send:", "don't care", NULL);
		return error("DELETE:", "no file:", ("upload/" + request.path).c_str()), 1;
	}
	if (send(connection, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"), 0) == -1)
		error("Send:", "don't care", NULL);
	return 1;
}