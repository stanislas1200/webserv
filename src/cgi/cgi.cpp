#include "../../include/cgi.hpp"

std::map<std::string, std:string>	mapToMap(const std::map<std::string, std::string>& headers)
{
	std::map<std::string, std::string>				env;
	std::map<std::string, std::string>::iterator	it;

	for (it = headers.begin(); it != headers.end(); ++it)
	{
		if (it->first == "Accept")
			env.insert("HTTP_ACCEPT=", it->second);
		else if (it->first == "Accept-Encoding")
			env.insert("HTTP_ACCEPT_ENCODING=", it->second);
		else if (it->first == "Accept-Language")
			env.insert("HTTP_ACCEPT_LANGUAGE=", it->second);
		else if (it->first == "Connection")
			env.insert("HTTP_CONNECTION=", it->second);
		else if (it->first == "Content-Length")
			env.insert("CONTENT_LENGTH=", it->second);
		else if (it->first == "Content-Type")
			env.insert("CONTENT_TYPE=", it->second);
		else if (it->first == "Host")
			env.insert("HTTP_HOST=", it->second);
		else if (it->first == "User-Agent")
			env.insert("HTTP_USER_AGENT=", it->second);
	}
	env.insert("SERVER_PROTOCOL=", "HTTP/1.1");
	return (env);
}

std::string	getOutput(int fd)
{
	std::string	outputString == "";
	char		buff[1024];
	int			check;

	while (check = read(fd, buff, 1024))
	{
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

std::string	runCgi(request& request)
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
		std::map<std::string, std::string> env = mapToMap(request.headers);
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		if (execve(request.path.c_str(), NULL, env.data()) == -1)
		{
			std::cerr << RED "CGI execve failed" C << std::endl;
			std::exit(1);
		}
	}

	/***	PARENT	***/
	std::string	outputString;
	int			childStatus;

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
