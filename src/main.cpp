#include "../include/Webserv.hpp"

void error(const char *type, const char *msg, const char *bold) {
	std::cerr << RED "Error: " YELLOW << type << " " C << msg;
	if (bold)
		std::cerr << DV " " << bold << C;
	std::cerr << C << std::endl;
	// exit(1);
}

void getConfig(std::vector<ServConfig> *configClass, std::string file) {
	std::ifstream confFile(file.c_str());
	ServConfig  newElement;

    try {
        while (confFile.eof() != 1) {
            newElement.initializeConfig(&confFile);
            configClass->push_back(newElement);
		}
	}
    catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
    confFile.close();
}

bool stringEnd(std::string file, std::string end) {
	if (file.size() < end.size())
		return false;

	int j = end.size();
	for (unsigned long i = file.size(); i > file.size() - end.size(); i--)
	{
		if (file[i] != end[j])
			return false;
		j--;
	}
	return true;
}

void	sendFile(int connection, std::ifstream *file, std::string status, std::string fileName) {

	std::string responce;
	std::stringstream ss;
	if (stringEnd(fileName, ".html"))
	{
		std::ifstream templat("src/pages/template.html");
		ss << templat.rdbuf();
		responce = ss.str();
		
		// Get response content
		ss.str(""); // emtpy
		ss << file->rdbuf();
		std::string fileContent = ss.str();

		size_t pos = responce.find("{{BODY}}");
		if (pos != std::string::npos) {
			if (file->is_open())
				responce.replace(pos, pos + 8, fileContent);
			else
				responce.replace(pos, pos + 8, "<h1 style=\"text-align:center\">Error 404 Not Found Error</h1>");
		}
		else
		{
			status = "500";
			responce = "<h1 style=\"text-align:center\">Error 500 Internal Server Error</h1>";
			error("File:", "Error in template file", NULL);
		}
		templat.close();
	}
	else
	{
		ss << file->rdbuf();
		responce = ss.str();
	}
	// Send status line
	std::string statusLine = "HTTP/1.1 " + status + " OK\r\n"; // TODO : map status code and message
	send(connection, statusLine.c_str(), statusLine.length(), 0);
    send(connection, "Content-Type: text/html\r\n\r\n", strlen("Content-Type: text/html\r\n\r\n"), 0);

	send(connection, responce.c_str(), responce.size(), 0);

	file->close();
}

int main(int ac, char **av) {
    std::vector<ServConfig> configClass;

	if (ac != 2) {
		// error("Usage:", av[0], "<config_file>");
		std::cout << GREEN "WebServ: " << MB "Using default config file" C << std::endl;
		getConfig(&configClass, "configs/default");
	}
	else {
		getConfig(&configClass, av[1]);
	}
	for (std::vector<ServConfig>::iterator it = configClass.begin(); it != configClass.end(); it++) {
		std::cout << *it << std::endl;
	}
	acceptConnection(configClass);
}