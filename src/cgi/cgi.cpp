#include "../../include/cgi.hpp"

std::vector<char *>	mapConvert(std::map<std::string, std::string>& headers)
{
	// std::map<std::string, std::string>				env;

	// for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
	// {
	// 	if (it->first == "Accept")
	// 		env["HTTP_ACCEPT="] = it->second;
	// 	else if (it->first == "Accept-Encoding")
	// 		env["HTTP_ACCEPT_ENCODING="] = it->second;
	// 	else if (it->first == "Accept-Language")
	// 		env["HTTP_ACCEPT_LANGUAGE="] = it->second;
	// 	else if (it->first == "Connection")
	// 		env["HTTP_CONNECTION="] = it->second;
	// 	else if (it->first == "Content-Length")
	// 		env["CONTENT_LENGTH="] = it->second;
	// 	else if (it->first == "Content-Type")
	// 		env["CONTENT_TYPE="] = it->second;
	// 	else if (it->first == "Host")
	// 		env["HTTP_HOST="] = it->second;
	// 	else if (it->first == "User-Agent")
	// 		env["HTTP_USER_AGENT="] = it->second;
	// }
	// env["SERVER_PROTOCOL="] = "HTTP/1.1";

	std::vector<char *>	env;

	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
	{
		if (it->first == "Accept")
			env.push_back(const_cast<char*>(("HTTP_ACCEPT=" + it->second).c_str()));
		else if (it->first == "Accept-Encoding")
			env.push_back(const_cast<char*>(("HTTP_ACCEPT_ENCODING=" + it->second).c_str()));
		else if (it->first == "Accept-Language")
			env.push_back(const_cast<char*>(("HTTP_ACCEPT_LANGUAGE=" + it->second).c_str()));
		else if (it->first == "Connection")
			env.push_back(const_cast<char*>(("HTTP_CONNECTION=" + it->second).c_str()));
		else if (it->first == "Content-Length")
			env.push_back(const_cast<char*>(("CONTENT_LENGTH=" + it->second).c_str()));
		else if (it->first == "Content-Type")
			env.push_back(const_cast<char*>(("CONTENT_TYPE=" + it->second).c_str()));
		else if (it->first == "Host")
			env.push_back(const_cast<char*>(("HTTP_HOST=" + it->second).c_str()));
		else if (it->first == "User-Agent")
			env.push_back(const_cast<char*>(("HTTP_USER_AGENT=" + it->second).c_str()));
	}
	env.push_back(const_cast<char*>("SERVER_PROTOCOL=HTTP/1.1"));
	env.push_back(NULL);
	return (env);
}

// const char**	vectorToArray(std::vector<std::string> env)
// {
// 	int		size = env.size();
// 	char	**argv = new const char*[size + 1];

// 	for (int j = 0; j < size; ++j)
// 			argv[j] = env[j].c_str();
// 	argv [size + 1] = NULL;
// 	return (argv);
// }

std::string	getOutput(int fd)
{
	std::string	outputString = "";
	char		buff[1024];
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

std::string	runCgi(s_request& request)
{
	// if (request.method == 'POST')
	// {
	// 	std::cout << request.body << std::endl;
	// }
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
		std::vector<char *> env = mapConvert(request.headers);
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

