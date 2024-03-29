#include "../include/Webserv.hpp"

void error(const char *type, const char *msg, const char *bold) {
	std::cerr << RED "Error: " YELLOW << type << " " C << msg;
	if (bold)
		std::cerr << DV " " << bold << C;
	std::cerr << C << std::endl;
	// exit(1);
}

void getConfig(s_config *config, std::string file) {
	std::ifstream confFILE(file.c_str());
	if (!confFILE.is_open()) {
		error("Config:", strerror(errno), NULL);
		return ;
	}

	std::string line;
	while (std::getline(confFILE, line)) {
		std::cout << line << std::endl;
		if (line.find("listen") != std::string::npos) {
			config->port = atoi(line.substr(line.find(" ") + 1).c_str());
			if (config->port < 0 || config->port > 65535)
				error("Port:", "Invalid port number", NULL);
		}
	}
    confFILE.close();

	std::ifstream confFile(file.c_str());
    ServConfig  configClass;
    try {
        configClass.initializeConfig(&confFile);
    }
    catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}

void	sendFile(int connection, std::string fileContent, std::string status) {

	std::ifstream templat("src/pages/template.html");
	std::stringstream ss;
	ss << templat.rdbuf();
	std::string templateStr = ss.str();

	size_t pos = templateStr.find("{{BODY}}");
	if (pos != std::string::npos) {
		templateStr.replace(pos, pos + 8, fileContent);}
	else
	{
		status = "500";
		templateStr = "<h1 style=\"text-align:center\">Error 500 Internal Server Error</h1>";
		error("File:", "Error in template file", NULL);
	}
	// Send status line
	std::string statusLine = "HTTP/1.1 " + status + " OK\r\n"; // TODO : map status code and message
	send(connection, statusLine.c_str(), statusLine.length(), 0);
    send(connection, "Content-Type: text/html\r\n\r\n", strlen("Content-Type: text/html\r\n\r\n"), 0);

	send(connection, templateStr.c_str(), templateStr.size(), 0);

	templat.close();
}

int main(int ac, char **av) {
	s_config config;

	if (ac != 2) {
		// error("Usage:", av[0], "<config_file>");
		std::cout << GREEN "WebServ: " << MB "Using default config file" C << std::endl;
		getConfig(&config, "configs/default");
	}
	else
		getConfig(&config, av[1]);

	acceptConnection(config);
}