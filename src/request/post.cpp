#include "../../include/request.hpp"

#include <algorithm>

#include <cctype>

// Function to check if a string has leading or trailing whitespace
bool hasLeadingOrTrailingWhitespace(const std::string& str) {
    if (str.empty()) {
        return false; // Empty string has no whitespace
    }
    
    // Check leading whitespace
    if (std::isspace(str.front())) {
        return true; // Leading whitespace found
    }
    
    // Check trailing whitespace
    if (std::isspace(str.back())) {
        return true; // Trailing whitespace found
    }
    
    return false; // No leading or trailing whitespace
}

void trimWhitespace(std::string& str) {
    // Remove leading whitespace
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start != std::string::npos) {
        str = str.substr(start);
    } else {
        str.clear(); // String contains only whitespace
        return;
    }

    // Remove trailing whitespace
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    if (end != std::string::npos) {
        str = str.substr(0, end + 1);
    }
}

void printFile(s_request *request, bool body) {
	std::cout << DV "Header:\n" MB << request->formData[0].header << std::endl;
	if (body)
	{
		std::cout << DV "Body:" MB << std::endl;
		// for (int i = 0; i < request->formData[0].bodySize; i++) {
		for (size_t i = 0; i < request->formData[0].data.size(); i++) {
			std::cout << request->formData[0].data[i];
		}
	}
	std::cout << RED << "-----PRINT END-----" << std::endl;
}

std::string parseFormData(s_request *request) { // FIXME : response to each file uploaded
	// return("200");
	s_FormDataPart *formDataPart = &request->formData[0];
	// size_t pos = 0;
	std::istringstream iss(formDataPart->header);
	std::string line;
	std::string boundaryEnd = request->boundary + "--";

	// if (formDataPart->data.size() > boundaryEnd.size())
	// {
	// 	std::vector<char>::iterator bpos;
	// 	if ((bpos = std::search(formDataPart->data.begin() + request->boundary.size(), formDataPart->data.end(), boundaryEnd.begin(), boundaryEnd.end())) != formDataPart->data.end())
	// 	{
	// 		std::cout << GREEN << "FIND" << std::endl;
	// 		formDataPart->data.erase(bpos, formDataPart->data.end());
	// 	}
	// 	else
	// 		std::cout << GREEN << "NO FIND" << std::endl;
	// }

	printFile(request, true);

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

	std::cout << MB << formDataPart->name << C << std::endl;
	std::ofstream outputFile(("upload/" + formDataPart->filename).c_str(), std::ios::binary);
	if (!outputFile.is_open())
	{
		error("Open:", strerror(errno), NULL);
		send(request->connection, "File upload failed! ", strlen("File upload failed! "), 0);
		return "500";
	}
	
	// pos = formDataPart->header.find("\r\n\r\n");
	if (request->dataLen >= stoul(request->headers["Content-Length"]))
	{
		// std::vector<char>::iterator bpos;
		// if (hasLeadingOrTrailingWhitespace(request->boundary))
		// {
		// 	std::cout << YELLOW << request->boundary << C << std::endl;
		// 	trimWhitespace(request->boundary);
		// }

		// // std::cout << formDataPart->data.size() << std::endl;
		// formDataPart->data.erase(formDataPart->data.begin() + stoul(request->headers["Content-Length"]), formDataPart->data.end());
		// // std::cout << formDataPart->data.size() << std::endl;


		// if ((bpos = std::search(formDataPart->data.begin() + request->boundary.size(), formDataPart->data.end(), boundaryEnd.begin(), boundaryEnd.end())) != formDataPart->data.end())
		// {
		// 	std::cout << YELLOW << "FIND>" << std::endl;
		// 	formDataPart->data.erase(bpos, formDataPart->data.end()); // FIXME : don't erase boundary or write to much ?
		// }
		// else
		// 	std::cout << YELLOW << "NO FIND" << std::endl;

		// // for (size_t i = pos + 4; i < formDataPart->data.size(); i++)
		// // 	std::cout << &formDataPart->data[i];
		outputFile.write(&formDataPart->data[0], formDataPart->data.size());
	}
	else
		outputFile.write(&formDataPart->data[0], formDataPart->data.size());
	std::cout << MB "File uploaded!" << std::endl;
	outputFile.close();
	handleGetRequest(request->connection, *request);
	send(request->connection, "File uploaded! ", strlen("File uploaded! "), 0);
	return "200";
}

int readFormData(s_request *request) {
	size_t bufferSize = 100000;
	char buffer[bufferSize + 1];
	s_FormDataPart *formDataPart = &request->formData[0];
	size_t bytes = 0;
	size_t pos = 0;

	if (request->boundary.empty())
	{
		pos = request->headers["Content-Type"].find("boundary=") + 9; // TODO check it exists		
		request->boundary = "--" + request->headers["Content-Type"].substr(pos, request->headers["Content-Type"].size() - pos - 1);
		std::cout << YELLOW << request->boundary << C << std::endl;
	}

	if ((bytes = recv(request->connection, buffer, bufferSize, 0)) > 0 && bytes != std::string::npos)
	{
		std::cout << "byte: " << bytes << std::endl;
		buffer[bytes] = '\0';
		// if ((pos = formDataPart->header.find("\r\n\r\n")) == std::string::npos)
		// 	formDataPart->header += buffer;
		request->dataLen += bytes;
		formDataPart->data.insert(formDataPart->data.end(), buffer, buffer + bytes);

		if (formDataPart->data.size() > request->boundary.size())
		{
			std::vector<char>::iterator bpos;
			if ((bpos = std::search(formDataPart->data.begin() + request->boundary.size(), formDataPart->data.end(), request->boundary.begin(), request->boundary.end())) != formDataPart->data.end())
			{
				std::cout << GREEN << "[FORMDATA-READ] FIND BOUNDARY" << std::endl;
				// NEXT HEADER
				// request->formData[1].header.clear();
				// request->formData[1].header = std::string(bpos, formDataPart->data.end());
				// NEXT BODY
				request->formData[1].data.clear();
				request->formData[1].data.insert(request->formData[1].data.end(), bpos, formDataPart->data.end());

				// HEADER
				const char *crlf2 = "\r\n\r\n";
				std::string head(formDataPart->data.begin(), std::search(formDataPart->data.begin(), formDataPart->data.end(), crlf2, crlf2 + 4)) ;
				formDataPart->header = head;
				// BODY 
				formDataPart->data.erase(bpos, formDataPart->data.end()); // next file data
				// std::vector<char> body(std::search(formDataPart->data.begin(), formDataPart->data.end(), crlf2, crlf2 + 4), formDataPart->data.end());
				// formDataPart->data = body;
				formDataPart->data.erase(formDataPart->data.begin(), std::search(formDataPart->data.begin(), formDataPart->data.end(), crlf2, crlf2 + 4) + 4);
				
				formDataPart->full = true;
				if (request->dataLen < stoul(request->headers["Content-Length"]))
				{
					parseFormData(request);
					return 0; // chunk
				}
				return 1;
			}
			else
				std::cout << RED << "[FORMDATA-READ] NOT FIND BOUNDARY" << std::endl;
		}

		if (request->dataLen < stoul(request->headers["Content-Length"]))
			return 0; // chunk
	}
	std::cout << formDataPart->header << std::endl;
	return 1;
}

std::string handleFormData(int connection, s_request *request, int *end) {

	// std::cout << DV "handleFormData" << std::endl;

	// s_FormDataPart *formDataPart = &request->formData;
	// std::cout << MB << formDataPart->data.size() << C << std::endl;
	// size_t bytes = 0;
	// size_t pos = request->headers["Content-Type"].find("boundary=") + 9;
	// request->boundary = "--" + request->headers["Content-Type"].substr(pos);
	// char buffer[10024];

	// // timeout
	// constexpr int timeout = 0.0001;
	
	// std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

	// // read on socket // TODO : handle multiple files
	// std::cout << YELLOW << request->boundary << C << std::endl;
	// *end = 0;
	// // std::cout << RED << connection << std::endl; // FIXME : connection broken at continue
	// // std::cout << RED << request->connection << std::endl;
	// // std::cout << YELLOW << pos << C << std::endl;
	// size_t bufferSize = 500;
	// while ((bytes = recv(request->connection, buffer, bufferSize, 0)) > 0 && bytes != std::string::npos)
	// {
	// 	buffer[bytes] = '\0';
	// 	// if (header.find(request->boundary) != std::string::npos)
	// 	// {
	// 	// 	std::cout << RED << "FIND" << std::endl;
	// 	// }
	// 	if ((pos = request->formData.header.find("\r\n\r\n")) == std::string::npos)
	// 		request->formData.header += buffer;
		
	// 	request->formData.dataLen += bytes;
	// 	formDataPart->data.insert(formDataPart->data.end(), buffer, buffer + bytes);

	// 	if (stoul(request->headers["Content-Length"]) == formDataPart->data.size())
	// 		break;
	// 	else
	// 	{
	// 		std::cout << RED << "Chunk" << std::endl;
	// 		*end = 0;
	// 		return "Chunked";
	// 	}

	// 	if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count() > timeout)
	// 	{
	// 		std::cout << DV << "size: " << request->formData.dataLen << C << std::endl;
	// 		error("Timeout:", "no end of request", NULL);
	// 		// send(request->connection, "Timeout: no end of request", strlen("Timeout: no end of request"), 0);
	// 		return "500";
	// 	}
	// }
	(void)connection;
	if (request->formData[0].full)
	{
		request->formData[0].header.clear();
		request->formData[0].data.clear();
		request->formData[0].full = false;
		request->formData[0] = request->formData[1];
	}
	size_t pos = 0;
	*end = readFormData(request);
	std::cout << *end << std::endl;
	if (*end == 0)
		return "Chunked";
	return parseFormData(request);

	// parse header
	s_FormDataPart *formDataPart = &request->formData[0];
	std::istringstream iss(formDataPart->header);
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
	pos = formDataPart->header.find("\r\n\r\n");
	std::cout << YELLOW << pos << C << std::endl;
	outputFile.write(&formDataPart->data[pos + 4], request->dataLen- (pos + 4) - request->boundary.size()); // +2 if end request (--)
	outputFile.close();
	handleGetRequest(request->connection, *request);
	send(request->connection, "File uploaded! ", strlen("File uploaded! "), 0);
	std::cout << MB "File uploaded!" << std::endl;
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
	int end = 1;

	std::cout << DV << "POST request" << std::endl;

	if (request->headers["Content-Type"].find("multipart/form-data") != std::string::npos)
		status = handleFormData(connection, request, &end);
	else if (request->headers["Content-Type"].find("application/x-www-form-urlencoded") != std::string::npos)
		status = handleUrlEncoded(connection, *request);
	else if (request->headers["Content-Type"].find("application/json") != std::string::npos)
		status = handleJson(connection, *request);
	std::cout << DV << "POST request end" << std::endl;
	std::string response = "HTTP/1.1 " + status + " OK\r\n"; // TODO : map status code and message
	if (end)
		send(connection, response.c_str(), response.length(), 0);
	
	std::cout << DV "PARSE REQUEST END" << std::endl;
	return end;
}
