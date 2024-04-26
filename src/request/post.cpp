#include "../../include/request.hpp"

#include <algorithm>
std::vector<unsigned char>	runCgi(s_request& request);
void requestCgi(s_request& request);

bool is_number(const std::string& s)
{
	std::string::const_iterator it = s.begin();
	while (it != s.end() && std::isdigit(*it)) ++it;
	return !s.empty() && it == s.end();
}

void printFile(s_request *request, bool body) {
	std::cout << DV "Header:\n" MB << request->formData[0].header << std::endl;
	if (body)
	{
		std::cout << DV "Body:" MB << std::endl;
		for (size_t i = 0; i < request->formData[0].data.size(); i++) {
			std::cout << request->formData[0].data[i];
		}
	}
	std::cout << RED << "-----PRINT END-----" << std::endl;
}

int parseFormData(s_request *request) {
	s_FormDataPart *formDataPart = &request->formData[0];
	std::istringstream iss(formDataPart->header);
	std::string line;
	std::string boundaryEnd = request->boundary + "--";

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
	
	std::ofstream outputFile(("upload/" + formDataPart->filename).c_str(), std::ios::binary);
	if (!outputFile.is_open() || !formDataPart->data.size())
	{
		error("Open:", strerror(errno), NULL);
		return 500;
	}

	outputFile.write(&formDataPart->data[0], formDataPart->data.size());
	std::cout << MB "File uploaded!" << std::endl;
	outputFile.close();
	return 200;
}

int chunckData(s_request *request, s_FormDataPart *formDataPart) {
	size_t length = std::strtoul (request->headers["Content-Length"].c_str(), NULL, 10);

	if (request->dataLen > request->maxBody)
		return -3;
		
	if (formDataPart->data.size() > request->boundary.size())
	{
		std::vector<char>::iterator bpos;
		if ((bpos = std::search(formDataPart->data.begin() + request->boundary.size(), formDataPart->data.end(), request->boundary.begin(), request->boundary.end())) != formDataPart->data.end())
		{
			// check multiple file
			int boundaryCount = 0;
			std::vector<char>::iterator cbpos = bpos;
			while ((cbpos = std::search(cbpos, formDataPart->data.end(), request->boundary.begin(), request->boundary.end())) != formDataPart->data.end())
			{
				++boundaryCount;
				// Move the iterator past the current found boundary for the next search
				cbpos += request->boundary.size();
			}
			std::cout << boundaryCount << std::endl;

			std::cout << GREEN << "[FORMDATA-READ] FIND BOUNDARY" << std::endl;

			// NEXT BODY
			request->formData[1].data.clear();
			request->formData[1].data.insert(request->formData[1].data.end(), bpos, formDataPart->data.end());

			// HEADER
			const char *crlf2 = "\r\n\r\n";
			std::string head(formDataPart->data.begin(), std::search(formDataPart->data.begin(), formDataPart->data.end(), crlf2, crlf2 + 4)) ;
			formDataPart->header = head;
			// BODY 
			formDataPart->data.erase(bpos, formDataPart->data.end()); // next file data

			if (formDataPart->data.size() > sizeof(crlf2))
			{
				std::vector<char>::iterator pos = std::search(formDataPart->data.begin(), formDataPart->data.end(), crlf2, crlf2 + 4);
				if (pos == formDataPart->data.end())
					return -2;
				formDataPart->data.erase(formDataPart->data.begin(), pos + 4);
			}
			
			formDataPart->full = true;
			if (request->dataLen < length || boundaryCount > 1)
			{
				parseFormData(request);
				std::cout << RED "Unfinished upload" C<< std::endl;
				return 0; // chunk
			}
			std::cout << GREEN "Finished upload" C << std::endl;
			return 1;
		}
		else
			std::cout << RED << "[FORMDATA-READ] NOT FIND BOUNDARY" << std::endl;
	}

	if (request->dataLen < length)
		return 0; // chunk
	return 1;
}

int readFormData(s_request *request) {
	size_t bufferSize = 10000;
	char buffer[bufferSize + 1];
	s_FormDataPart *formDataPart = &request->formData[0];
	size_t bytes = 0;
	size_t pos = 0;

	if (request->boundary.empty())
	{
		pos = request->headers["Content-Type"].find("boundary=") + 9;
		if (pos == std::string::npos)
			return -2;	
		request->boundary = "--" + request->headers["Content-Type"].substr(pos, request->headers["Content-Type"].size() - pos - 1);
	}

	if ((bytes = recv(request->connection, buffer, bufferSize, 0)) > 0 && bytes != std::string::npos) 
	{
		std::cout << "byte: " << bytes << std::endl;
		buffer[bytes] = '\0';
		request->dataLen += bytes;
		formDataPart->data.insert(formDataPart->data.end(), buffer, buffer + bytes);

	}

	if (bytes == (size_t)-1 || bytes == std::string::npos)
	{
		std::cout << RED << "Error read: " << strerror(errno) << C << std::endl;
		return -1;
	}
	
	return chunckData(request, formDataPart);
}

int handleFormData(int connection, s_request *request, int *end) {
	std::cout << C"[" GREEN "handlePostRequest" C "] " << MB "formdata" C << std::endl;
	(void)connection;
	if (request->formData[0].full)
	{
		request->formData[0].header.clear();
		request->formData[0].data.clear();
		request->formData[0].full = false;
		request->formData[0] = request->formData[1];
	}
	// size_t pos = 0;
	*end = readFormData(request);
	if (*end == -1)
		return 500;
	if (*end == -2)
		return 400;
	if (*end == -3)
		return 413;
	if (*end == 0)
		return 200;
	return parseFormData(request);
}

int handleUrlEncoded(int connection, s_request request) {
	std::cout << C"[" GREEN "handlePostRequest" C "] " << MB "UrlEncoded" C << std::endl;
	
	char buffer[1000];
	size_t bytes = 0;
	size_t length = std::strtoul (request.headers["Content-Length"].c_str(), NULL, 10);
	std::string data;

	// read on socket
	while (length > 0 && (bytes = recv(connection, buffer, 999, 0)) > 0 && bytes != std::string::npos)
	{
		buffer[bytes] = '\0';
		data += buffer;
		length -= bytes;
		request.dataLen += bytes;
		if (request.dataLen > request.maxBody)
			return 413;
	}
	if (bytes == (size_t)-1 || bytes == std::string::npos)
		return 500;

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

	// write data in cout
	// std::map<std::string, std::string>::const_iterator it;
	// for (it = parsedData.begin(); it != parsedData.end(); ++it)
	// {
	// 	std::cout << MB << it->first << C ": " DV << it->second << std::endl;
		// send(connection, it->first.c_str(), it->first.length(), 0);
		// send(connection, ": ", 2, 0);
		// send(connection, it->second.c_str(), it->second.length(), 0);
		// send(connection, " ", 1, 0);
	// }
	return 200;
}

// std::string handleJson(int connection, s_request request) {
// 	std::cout << DV "handleJson" << std::endl;
// 	(void)connection;
// 	(void)request;
// 	return "200";
// }

int handlePostRequest(int connection, s_request *request) {
	std::cout << C"[" GREEN "handlePostRequest" C "] " << YELLOW "---START---" C << std::endl;
	if (request->headers["Content-Length"].empty())
	{
		sendError(411, *request);
		return 1;
	}
	else if (is_number(request->headers["Content-Length"]))
	{
		sendError(400, *request);
		return 1;
	}

	int status = 505;
	int end = 1;

	std::string path = request->loc.getRedirection();
	if (path.substr(0, request->loc.getPathToCgi().size()) == request->loc.getPathToCgi() && request->loc.getExCgi().find(path.substr(path.find_last_of('.') + 1)) != std::string::npos) {
		std::cout << C"[" GREEN "handlePostRequest" C "] " << YELLOW "CGI" C << std::endl;
		size_t length = std::strtoul (request->headers["Content-Length"].c_str(), NULL, 10);
		// read body
		char buffer[1000];
		size_t bytes = 0;

		while (length > 0 && (bytes = recv(connection, buffer, 999, 0)) > 0 && bytes != std::string::npos) // TODO : function read with chunck for all
		{
			buffer[bytes] = '\0';
			request->body += buffer;
			length -= bytes;
			request->dataLen += bytes;

			if (request->dataLen > request->maxBody)
				return sendError(413, *request), 1;
		}

		if (bytes == (size_t)-1 || bytes == std::string::npos)
			return 1;
			
		request->path = path;
		return requestCgi(*request), 1;
	}

	if (request->headers["Content-Type"].find("multipart/form-data") != std::string::npos)
		status = handleFormData(connection, request, &end);
	else if (request->headers["Content-Type"].find("application/x-www-form-urlencoded") != std::string::npos)
		status = handleUrlEncoded(connection, *request);
	// else if (request->headers["Content-Type"].find("application/json") != std::string::npos)
	// 	status = handleJson(connection, *request);
	std::cout << DV << "POST request end" << end << std::endl;
	if (end)
	{
		std::string response = responseHeader(status, *request);
		if (send(connection, response.c_str(), response.length(), 0) == -1)
			error("Send:", "don't care", NULL);
	}
	
	std::cout << C"[" GREEN "handlePostRequest" C "] " << YELLOW "---END---" C << std::endl;
	return end;
}
