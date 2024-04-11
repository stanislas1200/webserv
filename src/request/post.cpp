#include "../../include/request.hpp"

#include <algorithm>


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

std::string parseFormData(s_request *request) { // FIXME : response to each file uploaded
	s_FormDataPart *formDataPart = &request->formData[0];
	std::istringstream iss(formDataPart->header);
	std::string line;
	std::string boundaryEnd = request->boundary + "--";

	printFile(request, false);

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
	if (!outputFile.is_open())
	{
		error("Open:", strerror(errno), NULL);
		send(request->connection, "File upload failed! ", strlen("File upload failed! "), 0);
		return "500";
	}

	// TODO : check if need endbound
	outputFile.write(&formDataPart->data[0], formDataPart->data.size());
	std::cout << MB "File uploaded!" << std::endl;
	outputFile.close();
	handleGetRequest(request->connection, *request);
	send(request->connection, "File uploaded! ", strlen("File uploaded! "), 0);
	return "200";
}

int chunckData(s_request *request, s_FormDataPart *formDataPart) {
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
			formDataPart->data.erase(formDataPart->data.begin(), std::search(formDataPart->data.begin(), formDataPart->data.end(), crlf2, crlf2 + 4) + 4);
			
			formDataPart->full = true;
			std::cout << request->dataLen << " " << request->headers["Content-Length"] << std::endl;
			if (request->dataLen < stoul(request->headers["Content-Length"]) || boundaryCount > 1) // TODO : split file
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

	if (request->dataLen < stoul(request->headers["Content-Length"]))
		return 0; // chunk
	return 1; // TODO : idk but check this function
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

	if ((bytes = recv(request->connection, buffer, bufferSize, 0)) > 0 && bytes != std::string::npos) // TODO : check error
	{
		std::cout << "byte: " << bytes << std::endl;
		buffer[bytes] = '\0';
		request->dataLen += bytes;
		formDataPart->data.insert(formDataPart->data.end(), buffer, buffer + bytes);

		return chunckData(request, formDataPart);
	}
	
	return chunckData(request, formDataPart);
	return 1;
}

std::string handleFormData(int connection, s_request *request, int *end) {
	std::cout << C"[" GREEN "handlePostRequest" C "] " << MB "formdata" C << std::endl;
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
	std::ofstream outputFile(("upload/" + formDataPart->filename).c_str(), std::ios::binary);
	if (!outputFile.is_open())
	{
		error("Open:", strerror(errno), NULL);
		send(request->connection, "File upload failed! ", strlen("File upload failed! "), 0);
		return "500";
	}
	pos = formDataPart->header.find("\r\n\r\n");
	outputFile.write(&formDataPart->data[pos + 4], request->dataLen- (pos + 4) - request->boundary.size()); // +2 if end request (--)
	outputFile.close();
	handleGetRequest(request->connection, *request);
	send(request->connection, "File uploaded! ", strlen("File uploaded! "), 0);
	std::cout << C"[" GREEN "handlePostRequest" C "] " << MB "File uploaded!" C << std::endl;
	return "200";
}

std::string handleUrlEncoded(int connection, s_request request) {
	std::cout << C"[" GREEN "handlePostRequest" C "] " << MB "UrlEncoded" C << std::endl;
	
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
	std::cout << C"[" GREEN "handlePostRequest" C "] " << YELLOW "---START---" C << std::endl;
	std::string status = "505";
	int end = 1;


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
	
	std::cout << C"[" GREEN "handlePostRequest" C "] " << YELLOW "---END---" C << std::endl;
	return end;
}
