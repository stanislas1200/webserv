#include "../../include/request.hpp"

std::string handleFormData(int connection, s_request request) {

    std::vector<s_FormDataPart> formData;
	s_FormDataPart formDataPart;
	size_t bytes = 0;
	size_t pos = request.headers["Content-Type"].find("boundary=") + 9;
	std::string boundary = "--" + request.headers["Content-Type"].substr(pos);
	size_t dataLen = 0;
	std::string header;
	char buffer[10024];

	// read on socket // TODO : handle multiple files
    std::cout << YELLOW << boundary << C << std::endl;
	while ((bytes = recv(connection, buffer, 10000, 0)) > 0 && bytes != std::string::npos)
	{
		buffer[bytes] = '\0';
		if (header.find(boundary) != std::string::npos)
        {
            std::cout << RED << "FIND" << std::endl;
        }
		if ((pos = header.find("\r\n\r\n")) == std::string::npos)
			header += buffer;
		
		dataLen += bytes;
		formDataPart.data.insert(formDataPart.data.end(), buffer, buffer + bytes);		

		if (stoul(request.headers["Content-Length"]) == formDataPart.data.size())
			break;
	}

	// parse header
	std::istringstream iss(header);
	std::string line;
	while (std::getline(iss, line) && !line.empty())
	{
		if (line == "\r")
			break;
		if (line.find("Content-Disposition") != std::string::npos)
		{
			line = line.substr(line.find("name=\"") + 6);
			formDataPart.name = line.substr(0, line.find('"'));
			line = line.substr(line.find("filename=\"") + 10);
			formDataPart.filename = line.substr(0, line.find('"'));
		}
		if (line.find("Content-Type") != std::string::npos)
			formDataPart.contentType = line.substr(line.find("Content-Type:") + 14);
	}
	// TODO : check malformated request
	// write in file
	std::ofstream outputFile(("upload/" + formDataPart.filename).c_str(), std::ios::binary);
	if (!outputFile.is_open())
	{
		error("Open:", strerror(errno), NULL);
		send(connection, "File upload failed! ", strlen("File upload failed! "), 0);
		return "500";
	}
	std::cout << MB "File uploaded!" << std::endl;
	outputFile.write(&formDataPart.data[pos + 4], dataLen - (pos + 4) - boundary.size()); // +2 if end request (--)
	outputFile.close();
	handleGetRequest(connection, request);
	send(connection, "File uploaded! ", strlen("File uploaded! "), 0);
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

void handlePostRequest(int connection, s_request request) {
	std::string status = "505";

	if (request.headers["Content-Type"].find("multipart/form-data") != std::string::npos)
		status = handleFormData(connection, request);
	else if (request.headers["Content-Type"].find("application/x-www-form-urlencoded") != std::string::npos)
		status = handleUrlEncoded(connection, request);
	else if (request.headers["Content-Type"].find("application/json") != std::string::npos)
		status = handleJson(connection, request);

	std::string response = "HTTP/1.1 " + status + " OK\r\n"; // TODO : map status code and message
	send(connection, response.c_str(), response.length(), 0);
}
