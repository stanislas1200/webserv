#include "../../include/request.hpp"

std::string handleFormData(int connection, s_request *request, int *end) {

	std::cout << DV "handleFormData" << std::endl;

	s_FormDataPart *formDataPart = &request->formData;
	size_t bytes = 0;
	size_t pos = request->headers["Content-Type"].find("boundary=") + 9;
	std::string boundary = "--" + request->headers["Content-Type"].substr(pos);
	char buffer[10024];

	// timeout
	constexpr int timeout = 0.0001;
	
	std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

	// read on socket // TODO : handle multiple files
	std::cout << YELLOW << boundary << C << std::endl;
	*end = 0;
	std::cout << RED << connection << std::endl; // FIXME : connection broken at continue
	std::cout << RED << request->connection << std::endl;
	while ((bytes = recv(request->connection, buffer, 10000, 0)) > 0 && bytes != std::string::npos)
	{
		buffer[bytes] = '\0';
		// if (header.find(boundary) != std::string::npos)
		// {
		// 	std::cout << RED << "FIND" << std::endl;
		// }
		if ((pos = request->formData.header.find("\r\n\r\n")) == std::string::npos)
			request->formData.header += buffer;
		
		request->formData.dataLen += bytes;
		formDataPart->data.insert(formDataPart->data.end(), buffer, buffer + bytes);

		if (stoul(request->headers["Content-Length"]) == formDataPart->data.size())
			break;

		if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count() > timeout)
		{
			std::cout << DV << "size: " << request->formData.dataLen << C << std::endl;
			error("Timeout:", "no end of request", NULL);
			// send(request->connection, "Timeout: no end of request", strlen("Timeout: no end of request"), 0);
			return "500";
		}
	}
	*end = 1;

	// parse header
	std::istringstream iss(request->formData.header);
	std::string line;
	while (std::getline(iss, line) && !line.empty())
	{
		if (line == "\r")
			break;
		if (line.find("Content-Disposition") != std::string::npos)
		{
			line = line.substr(line.find("name=\"") + 6);
			formDataPart->name = line.substr(0, line.find('"'));
			line = line.substr(line.find("filename=\"") + 10);
			formDataPart->filename = line.substr(0, line.find('"'));
		}
		if (line.find("Content-Type") != std::string::npos)
			formDataPart->contentType = line.substr(line.find("Content-Type:") + 14);
	}
	// TODO : check malformated request
	// write in file
	std::cout << MB << formDataPart->filename << C << std::endl;
	std::ofstream outputFile(("upload/" + formDataPart->filename).c_str(), std::ios::binary);
	if (!outputFile.is_open())
	{
		error("Open:", strerror(errno), NULL);
		send(request->connection, "File upload failed! ", strlen("File upload failed! "), 0);
		return "500";
	}
	std::cout << MB "File uploaded!" << std::endl;
	outputFile.write(&formDataPart->data[pos + 4], request->formData.dataLen- (pos + 4) - boundary.size()); // +2 if end request (--)
	outputFile.close();
	handleGetRequest(request->connection, *request);
	send(request->connection, "File uploaded! ", strlen("File uploaded! "), 0);
	return "200";
}

std::string handleUrlEncoded(int connection, s_request request) {
	std::cout << DV "handleUrlEncoded" << std::endl;
	
	char buffer[1000];
	size_t bytes = 0;
	size_t length = stoul(request.headers["Content-Length"]);
	std::string data;

	// read on socket
	while (length > 0 && (bytes = recv(connection, buffer, 999, 0)) > 0 && bytes != std::string::npos)
	{
		buffer[bytes] = '\0';
		data += buffer;
		length -= bytes;
		std::cout << DV << length << MB << data.length() << std::endl;
	}

	// parse data
	std::map<std::string, std::string> parsedData;
	std::istringstream iss(data);
	std::string line;
	while (std::getline(iss, line, '&'))
	{
		std::istringstream lineStream(line);
		std::string key;
		std::string value;
		std::getline(lineStream, key, '=');
		std::getline(lineStream, value);
		parsedData[key] = value;
	}

	handleGetRequest(connection, request);

	// write data in cout
	std::map<std::string, std::string>::const_iterator it;
	for (it = parsedData.begin(); it != parsedData.end(); ++it)
	{
		std::cout << MB << it->first << C ": " DV << it->second << std::endl;
		send(connection, it->first.c_str(), it->first.length(), 0);
		send(connection, ": ", 2, 0);
		send(connection, it->second.c_str(), it->second.length(), 0);
		send(connection, " ", 1, 0);
	}
	return "200";
}

std::string handleJson(int connection, s_request request) {
	std::cout << DV "handleJson" << std::endl;
	(void)connection;
	(void)request;
	return "200";
}

int handlePostRequest(int connection, s_request *request) {
	std::string status = "505";
	int end;

	std::cout << DV << "POST request" << std::endl;

	if (request->headers["Content-Type"].find("multipart/form-data") != std::string::npos)
		status = handleFormData(connection, request, &end);
	else if (request->headers["Content-Type"].find("application/x-www-form-urlencoded") != std::string::npos)
		status = handleUrlEncoded(connection, *request);
	else if (request->headers["Content-Type"].find("application/json") != std::string::npos)
		status = handleJson(connection, *request);
	std::string response = "HTTP/1.1 " + status + " OK\r\n"; // TODO : map status code and message
	send(connection, response.c_str(), response.length(), 0);
	return end;
}
