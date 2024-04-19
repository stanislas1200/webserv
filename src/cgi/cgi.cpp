#include "../../include/cgi.hpp"

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
			env.push_back(cstr("HTTP_HOST=" + it->second.substr(1 , it->second.length() - 2))); // TODO : check -2 (\n)
		else if (it->first == "User-Agent")
			env.push_back(cstr("HTTP_USER_AGENT=" + it->second.substr(1)));
	}
	env.push_back(cstr("SERVER_PROTOCOL=HTTP/1.1"));
	env.push_back(cstr("REQUEST_METHOD=" + req.method));
	env.push_back(cstr("REQUEST_URI=/" + req.path));
	env.push_back(cstr("PATH_INFO=" + req.path)); // TODO : leave or fix
	env.push_back(NULL);
	return (env);
}

std::string	getOutput(int fd)
{
	std::string	outputString = "";
	char		buff[1025];
	int			check;

	while ((check = read(fd, buff, 1024)))
	{
		buff[check] = '\0';
		if (check == -1)
		{
			std::cerr << RED "CGI read failed" C << std::endl;
			// Throw error
			throw (tempThrow());
		}
		outputString.append(buff);
	}
	return (outputString);
}

std::string	runCgi(s_request& request) // TODO : if POST send body in standard input.
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
	std::string	outputString;
	int			childStatus;
	
	if (request.method == "POST")
		write(fd[1], request.body.data(), request.body.size());

	close(fd[1]);
	waitpid(pid, &childStatus, 0);
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
