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
        while (confFile.eof() != 1) {
			ServConfig  newElement;
            newElement.initializeConfig(&confFile);
            configClass->push_back(newElement);
		}
	}
    catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		exit(1);
	}
    confFile.close();
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
	for (std::vector<ServConfig>::iterator it = configClass.begin(); it != configClass.end(); it++) {
		std::cout << *it << std::endl;
	}
	std::cout << "Number of server: " << configClass.size() << std::endl;
	acceptConnection(configClass);
}