#include "../../include/request.hpp"

std::string readHeader(int connection) {
	std::string header;
	char buffer[1024];
	int bytes = 0;

	while ((bytes = recv(connection, buffer, 1, 0)) > 0) {
		buffer[bytes] = '\0';
		header += buffer;
		if (header.find("\r\n\r\n") != std::string::npos)
			break;
	}
	return header;
}

bool stringEnd(std::string file, std::string end) {
	if (file.size() < end.size())
		return false;

	int j = end.size();
	for (unsigned long i = file.size(); i > file.size() - end.size(); i--)
	{
		if (file[i] != end[j])
			return false;
		j--;
	}
	return true;
}

std::string responseHeader(int status)
{
	std::map<int, std::string> httpStatusCodes;
	httpStatusCodes[100] = "Continue";
	httpStatusCodes[101] = "Switching Protocols";
	httpStatusCodes[102] = "Processing";
	httpStatusCodes[103] = "Early Hints";
	httpStatusCodes[200] = "OK";
	httpStatusCodes[201] = "Created";
	httpStatusCodes[202] = "Accepted";
	httpStatusCodes[203] = "Non-Authoritative Information";
	httpStatusCodes[204] = "No Content";
	httpStatusCodes[205] = "Reset Content";
	httpStatusCodes[206] = "Partial Content";
	httpStatusCodes[300] = "Multiple Choices";
	httpStatusCodes[301] = "Moved Permanently";
	httpStatusCodes[302] = "Found";
	httpStatusCodes[303] = "See Other";
	httpStatusCodes[304] = "Not Modified";
	httpStatusCodes[305] = "Use Proxy";
	httpStatusCodes[307] = "Temporary Redirect";
	httpStatusCodes[400] = "Bad Request";
	httpStatusCodes[401] = "Unauthorized";
	httpStatusCodes[402] = "Payment Required";
	httpStatusCodes[403] = "Forbidden";
	httpStatusCodes[404] = "Not Found";
	httpStatusCodes[405] = "Method Not Allowed";
	httpStatusCodes[406] = "Not Acceptable";
	httpStatusCodes[407] = "Proxy Authentication Required";
	httpStatusCodes[408] = "Request Timeout";
	httpStatusCodes[409] = "Conflict";
	httpStatusCodes[410] = "Gone";
	httpStatusCodes[411] = "Length Required";
	httpStatusCodes[412] = "Precondition Failed";
	httpStatusCodes[413] = "Payload Too Large";
	httpStatusCodes[414] = "URI Too Long";
	httpStatusCodes[415] = "Unsupported Media Type";
	httpStatusCodes[416] = "Range Not Satisfiable";
	httpStatusCodes[417] = "Expectation Failed";
	httpStatusCodes[426] = "Upgrade Required";
	httpStatusCodes[500] = "Internal Server Error";
	httpStatusCodes[501] = "Not Implemented";
	httpStatusCodes[502] = "Bad Gateway";
	httpStatusCodes[503] = "Service Unavailable";
	httpStatusCodes[504] = "Gateway Timeout";
	httpStatusCodes[505] = "HTTP Version Not Supported";

	std::string response = "HTTP/1.0 " + std::to_string(status) + " " + httpStatusCodes[status] + "\r\nContent-type:text/html\r\n\r\n"; // TODO : Content-type
	return response;
}

void sendError(int status, s_request req)
{
	// check error pages
	std::string content = "<h1 style=\"text-align:center\">Error " + std::to_string(status) + "</h1>";
	if (!req.conf.pathToErrorPage(status).empty())
	{
		std::ifstream file(req.conf.pathToErrorPage(status));
		if (file.is_open())
		{
			// read error page
			std::stringstream ss;
			ss << file.rdbuf();
			content += ss.str();
			file.close();

			// check template
			content = useTemplate(content, req);
		}
	}
	std::string response = responseHeader(status) + content;
	std::cout << response << std::endl;
	send(req.connection, response.c_str(), response.length(), 0);
}

std::string useTemplate(std::string content, s_request request) {
	std::string ret;
	std::string path = request.path;
	std::stringstream ss;

	if (!request.loc.getTemplate().empty() && stringEnd(path, ".html"))
	{
		std::ifstream templat(request.loc.getTemplate());
		if (templat.is_open())
		{
			ss << templat.rdbuf();
			ret = ss.str();
			templat.close();

			size_t pos = ret.find("{{BODY}}");
			if (pos != std::string::npos) {
				ret.replace(pos, pos + 8, content);
				return ret;
			}
			// else
			// {
			// 	status = "500 KO\r\n";
			// 	ret = "<h1 style=\"text-align:center\">Error 500 Internal Server Error</h1>";
			// 	error("File:", "Error in template file", NULL);
			// }

		}
		// else
		// {
		// 	status = "404 KO\r\n";
		// 	ret.replace(pos, pos + 8, "<h1 style=\"text-align:center\">Error 404 Not Found Error</h1>");
		// }
	}
	// ret = "HTTP/1.1 " + status + "Content-Type: text/html\r\n\r\n" + ret; // TODO : map status code and message
	return content;
}

void	sendFile(int connection, std::ifstream *file, int status, s_request request) {

	std::stringstream ss;
	ss << file->rdbuf();

	std::string fileContent = ss.str();
	std::string responce = useTemplate(fileContent, request);
	responce = responseHeader(status) + responce;
	// if (responce.empty())
	// 	responce = "HTTP/1.1 " + status + "Content-Type: text/html\r\n\r\n" + fileContent; // TODO : map status code and message
	// Send status line
	// send(connection, statusLine.c_str(), statusLine.length(), 0);
    // send(connection, "Content-Type: text/html\r\n\r\n", strlen("Content-Type: text/html\r\n\r\n"), 0);
	file->close();
	send(connection, responce.c_str(), responce.size(), 0);

}

void printRequest(s_request request) {
	std::cout << GREEN "Method: " MB << request.method << std::endl;
	std::cout << GREEN "Path: " MB << request.path << std::endl;
	std::map<std::string, std::string>::const_iterator it;
	for (it = request.headers.begin(); it != request.headers.end(); ++it) {
		std::cout << MB << it->first << C ": " DV << it->second << std::endl;
	}
	std::cout << YELLOW "Body:\n" MB << request.body << std::endl;
}

std::string	replaceHexAndAmp(std::string src)
{
	std::string	str;
	int		 	hexValue;
	size_t 		length = src.length();
	
	for (size_t i = 0; i < length; ++i)
	{
		if (src[i] == '%' && i + 2 < length && isxdigit(src[i + 1]) && isxdigit(src[i + 2]))
		{
			sscanf(src.substr(i + 1, 2).c_str(), "%x", &hexValue);
			str += static_cast<char>(hexValue);
			i += 2;
		}
		else if (src[i] == '&')
			str += '\n';
		else
			str += src[i];
	}
	return (str);
}

int parseRequest(std::string header, s_request *request) {
	// Parse request header if needed
	int connection = request->connection;
	if (request->method.empty()) // TODO : split path?key=value
	{
		std::istringstream requestStream(header);
		std::string line;

		// Parse the first line of the request
		std::getline(requestStream, line);
		std::istringstream firstLineStream(line);
		firstLineStream >> request->method >> request->path;
		request->path = replaceHexAndAmp(request->path);

		// Parse the headers
		while (std::getline(requestStream, line) && line != "\r") {
			std::istringstream headerStream(line);
			std::string headerName;
			std::string headerValue;
			std::getline(headerStream, headerName, ':');
			std::getline(headerStream, headerValue);
			request->headers[headerName] = headerValue;
		}
	}
	// handle methode
	std::cout << C"[" DV "parseRequest" C "] " << MB "METHOD" C ": " GREEN << request->method << C << std::endl;
	// printRequest(*request);
	std::vector<Location> loc = request->conf.getLocation();
	std::string fileName;
	if (request->method == "DELETE")
	{
		fileName = request->path.substr(request->path.find_last_of("/"));
		request->path = request->path.substr(0, request->path.find_last_of("/"));
		if (request->path.empty())
			request->path = "/";
	}
	for (size_t i = 0; i < loc.size(); i++)
	{
		if (loc[i].getPath() == request->path)
		{
			request->loc = loc[i];
			if (request->method == "GET" && loc[i].getMethode().find("GET") != std::string::npos)
				return handleGetRequest(connection, *request);
			else if (request->method == "POST" && loc[i].getMethode().find("POST") != std::string::npos) // TODO : cgi ? cgi outside methode ?
				return handlePostRequest(connection, request);
			else if (request->method == "DELETE" && loc[i].getMethode().find("DELETE") != std::string::npos) // TODO : cgi
			{
				request->path += fileName;
				return handleDeleteRequest(connection, *request);
			}
			else
				return sendError(505, *request), 1;
		}
	}
	sendError(404, *request);
	return 1;
}
