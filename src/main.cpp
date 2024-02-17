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