#include "../../include/cgi.hpp"
#include "../../include/request.hpp"

char * cstr(std::string s)
{
	char *c = new char[s.size() + 1];
	strcpy(c, s.c_str());
	return c;
}

std::vector<char *>	mapConvert(std::map<std::string, std::string>& headers, s_request req)
{
	std::vector<char *>	env;

	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
	{
		if (it->second.length() >= 2 && (it->second[it->second.length() - 1] == '\r' || it->second[it->second.length() - 1] == '\n'))
			it->second = it->second.substr(0, it->second.length() - 1);

		if (it->first == "Accept")
			env.push_back(cstr("HTTP_ACCEPT=" + it->second.substr(1)));
		else if (it->first == "Accept-Encoding")
			env.push_back(cstr("HTTP_ACCEPT_ENCODING=" + it->second.substr(1)));
		else if (it->first == "Accept-Language")
			env.push_back(cstr("HTTP_ACCEPT_LANGUAGE=" + it->second.substr(1)));
		else if (it->first == "Connection")
			env.push_back(cstr("HTTP_CONNECTION=" + it->second.substr(1)));
		else if (it->first == "Content-Length")
			env.push_back(cstr("CONTENT_LENGTH=" + it->second.substr(1)));
		else if (it->first == "Content-Type")
			env.push_back(cstr("CONTENT_TYPE=" + it->second.substr(1)));
		else if (it->first == "Host")
			env.push_back(cstr("HTTP_HOST=" + it->second.substr(1 , it->second.length())));
		else if (it->first == "User-Agent")
			env.push_back(cstr("HTTP_USER_AGENT=" + it->second.substr(1)));
	}
	env.push_back(cstr("SERVER_PROTOCOL=HTTP/1.1"));
	env.push_back(cstr("REQUEST_METHOD=" + req.method));
	env.push_back(cstr("REQUEST_URI=/" + req.path));
	if (!req.queryString.empty())
		env.push_back(cstr("QUERY_STRING=" + req.queryString.substr(1)));
	env.push_back(cstr("PATH_INFO=" + req.path));
	env.push_back(NULL);
	return (env);
}

std::vector<unsigned char>	getOutput(int fd)
{
    std::vector<unsigned char> data;
	size_t bufferSize = 1024;
	char buffer[bufferSize + 1];
	int			check;

	while ((check = read(fd, &buffer, bufferSize)))
	{
		if (check == -1)
		{
			std::cerr << RED "CGI read failed" C << std::endl;
			// Throw error
			throw (tempThrow());
		}
		data.insert(data.end(), buffer, buffer + check);
	}
	return (data);
}

std::vector<unsigned char>	runCgi(s_request& request)
{
	int	fd[2], fdR[2];

	if (pipe(fd) == -1 || pipe(fdR) == -1)
	{
		std::cerr << RED "CGI pipe failed" C << std::endl;
		// Throw error
		request.status = 500;
		throw (tempThrow());
	}
	int pid = fork();
	if (pid == -1)
	{
		close(fd[0]);
		close(fd[1]);
		close(fdR[0]);
		close(fdR[1]);
		std::cerr << RED "CGI fork failed" C << std::endl;
		// Throw error
		request.status = 500;
		throw (tempThrow());
	}

	/***	CHILD	***/
	if (pid == 0)
	{
		std::vector<char *> env = mapConvert(request.headers, request);
		dup2(fdR[0], STDIN_FILENO);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		close(fdR[0]);
		close(fdR[1]);
		std::string fullPath = request.path;

		char *argv[] = {const_cast<char*>(fullPath.c_str()), NULL};
		if (execve(fullPath.c_str(), argv, env.data()) == -1)
		{
			for (std::vector<char *>::iterator it = env.begin(); it != env.end(); it++) {
				delete[] *it;
			}
			std::cerr << RED "CGI execve failed" C << std::endl;std::cerr << RED "CGI errno: " MB << strerror(errno) << C << std::endl;
			std::exit(1);
		}
	}

	/***	PARENT	***/
	std::vector<unsigned char>	outputString;
	int			childStatus;
	
	if (request.method == "POST" && !request.body.empty())
		write(fdR[1], request.body.data(), request.body.size());

	close(fd[1]);
	close(fdR[1]);
	close(fdR[0]);


	std::vector<unsigned char> data;
    char buffer[4096];
    ssize_t check;

	int timeout = request.conf.getTimeoutCgi();
	fcntl(fd[0], F_SETFL, O_NONBLOCK);
	while (waitpid(pid, &childStatus, WNOHANG) == 0)
	{

		// Read from pipe in chunks
		std::cout << "Reading from pipe" << std::endl;
		while ((check = read(fd[0], buffer, sizeof(buffer))) != 0 )
		{
			if (check != -1)
				data.insert(data.end(), buffer, buffer + check);
			if (timeout <= 0)
			{
				kill(pid, SIGTERM);
				error("CGI:", "timeout", NULL);
				request.status = 504;
				close(fd[0]);
				throw (tempThrow());
			}
			timeout--;
			sleep(1);
		}
		if (timeout <= 0) // not sure I need this
		{
			kill(pid, SIGTERM);
			error("CGI:", "timeout", NULL);
			request.status = 504;
			close(fd[0]);
			throw (tempThrow());
		}
		sleep(1);
		timeout--;
	}
	
	if (WIFEXITED(childStatus))
	{
		if (WEXITSTATUS(childStatus) != 0)
		{
			close(fd[0]);
			std::cerr << RED "CGI child return value != 0" C << std::endl;
			// Throw error
			request.status = 502;
			throw (tempThrow());
		}
	}
	// outputString = getOutput(fd[0]);
	outputString = data;
	close(fd[0]);
	return (outputString);
}

void sendChunk(int sockfd, const std::vector<unsigned char>& data, size_t chunkSize)
{
	for (size_t i = 0; i < data.size(); i += chunkSize)
	{
		// Get the next chunk of data
		std::vector<unsigned char> chunk(data.begin() + i, data.begin() + std::min(i + chunkSize, data.size()));

		// Send the chunk
		// int retries = 3;
		// while (retries > 0)
		// {
			if (send(sockfd, &chunk[0], chunk.size(), 0) == -1)
			{
				error("Send:", "don't care", NULL);
				return;
				// retries--;
				// if (retries == 0)
				// {
				// 	// Handle error
				// 	error("Send:", "Failed to send data after 3 attempts", NULL);
				// 	return;
				// }
				// // Sleep for a bit before retrying
				// sleep(1);
			}
		// 	else
		// 		break;
		// }
		usleep(1);
	}
}

void requestCgi(s_request& request)
{
	std::vector<unsigned char> response;
	try {
		request.status = 500;
		response = runCgi(request);
	} catch (const std::exception& e) {
		std::cerr << RED "Error: " YELLOW << e.what() << C << std::endl;
		return sendError(request.status, request);
	}
	std::string header = responseHeader(200, request);
	if (send(request.connection, header.c_str(), header.length(), 0) == -1)
		return error("Send:", "don't care", NULL);
	sendChunk(request.connection, response, 4096);
	// if (send(request.connection, response.data(), response.size(), 0) == -1)
	// 	error("Send:", "don't care", NULL);
}
