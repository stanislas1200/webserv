#include "../include/Webserv.hpp"

void error(const char *type, const char *msg, const char *bold) {
	std::cerr << RED "Error: " YELLOW << type << " " C << msg;
	if (bold)
		std::cerr << DV " " << bold << C;
	std::cerr << C << std::endl;
}

void getConfig(std::vector<ServConfig> *configClass, std::string file) {
	std::ifstream confFile(file.c_str());

    try {
		if (!confFile.is_open())
			throw std::runtime_error("Config file: Can't open file");
        while (confFile.eof() != 1) {
			ServConfig  newElement;
            if (newElement.initializeConfig(&confFile))
            	configClass->push_back(newElement);
		}
	}
    catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		exit(1);
	}
    confFile.close();
}

void	checkConfig(std::vector<ServConfig> configClass) {
	std::vector<int>	port;

	try {
		for (std::vector<ServConfig>::iterator it = configClass.begin(); it != configClass.end(); it++) {
			std::vector<int>::iterator its = std::find(port.begin(), port.end(), it->getPort());
			if (its != port.end())
				ServConfig::wrongFormatError("Multiple server", "have same port");
			port.push_back(it->getPort());
		}
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		exit(1);
	}
	for (std::vector<ServConfig>::iterator it = configClass.begin(); it != configClass.end(); it++) {
		std::cout << *it << std::endl;
	}
}

int main(int ac, char **av) {
    std::vector<ServConfig> configClass;

	if (ac != 2) {
		error("Usage:", av[0], "<config_file>");
		std::cout << GREEN "WebServ: " << MB "Using default config file" C << std::endl;
		getConfig(&configClass, "configs/default");
	}
	else {
		getConfig(&configClass, av[1]);
	}
	checkConfig(configClass);
	std::cout << "Number of server: " << configClass.size() << std::endl;
	acceptConnection(configClass);
}