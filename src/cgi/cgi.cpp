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
			it->second = it->second.substr(0, it->second.length() - 2);

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
	env.push_back(cstr("PATH_INFO=" + req.path)); // TODO : leave or fix
	env.push_back(NULL);
	return (env);
}

std::vector<unsigned char>	getOutput(int fd)
{
    std::vector<unsigned char> data;
	size_t bufferSize = 1024;
	char buffer[bufferSize + 1];
	int			check;
std::cout << "ok" << std::endl;
	while ((check = read(fd, &buffer, bufferSize)))
	{
		std::cout << "HEY" <<  check << std::endl;
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
	int	fd[2];

	if (pipe(fd) == -1)
	{
		std::cerr << RED "CGI pipe failed" C << std::endl;
		// Throw error
		throw (tempThrow());
	}
	int pid = fork();
	if (pid == -1)
	{
		close(fd[0]);
		close(fd[1]);
		std::cerr << RED "CGI fork failed" C << std::endl;
		// Throw error
		throw (tempThrow());
	}

	/***	CHILD	***/
	if (pid == 0)
	{
		std::vector<char *> env = mapConvert(request.headers, request); // TODO : free
		dup2(fd[0], STDIN_FILENO);
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		std::string fullPath = request.path;

		char *argv[] = {const_cast<char*>(fullPath.c_str()), NULL};
		if (execve(fullPath.c_str(), argv, env.data()) == -1)
		{
			std::cerr << RED "CGI execve failed" C << std::endl;std::cerr << RED "CGI errno: " MB << strerror(errno) << C << std::endl;
			std::exit(1);
		}
	}

	/***	PARENT	***/
	std::vector<unsigned char>	outputString;
	int			childStatus;
	
	if (request.method == "POST" && !request.body.empty())
		write(fd[1], request.body.data(), request.body.size());

	close(fd[1]);

	int timeout = 5;
	while (waitpid(pid, &childStatus, WNOHANG) == 0)
	{
		if (timeout <= 0)
		{
			kill(pid, SIGTERM);
			error("CGI:", "timeout", NULL);
			// TODO : return error
			// throw (tempThrow());
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
			throw (tempThrow());
		}
	}
	outputString = getOutput(fd[0]);
	close(fd[0]);
	return (outputString);
}

void requestCgi(s_request& request)
{
	std::vector<unsigned char> response;
	try {
		response = runCgi(request);
	} catch (const std::exception& e) {
		std::cerr << RED "Error: " YELLOW << e.what() << C << std::endl;
		return sendError(500, request);
	}
	std::string header = responseHeader(200);
	if (send(request.connection, header.c_str(), header.length(), 0) == -1)
		return error("Send:", "don't care", NULL);
	if (send(request.connection, response.data(), response.size(), 0) == -1)
		error("Send:", "don't care", NULL);
}
