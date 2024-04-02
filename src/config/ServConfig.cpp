/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServConfig.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gduchesn <gduchesn@students.s19.be>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 14:43:18 by gduchesn          #+#    #+#             */
/*   Updated: 2024/04/02 11:52:52 by gduchesn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Webserv.hpp"

ServConfig::ServConfig() : _name(""), _methode(""), _port(0), _maxClient(0) {}

ServConfig::~ServConfig() {}

ServConfig::ServConfig(const ServConfig &src) {
    _name = src._name;
    _methode = src._methode;
    _port = src._port;
    _maxClient = src._maxClient;
    _location = src._location;
    _errorpages = src._errorpages;
}

ServConfig& ServConfig::operator=(const ServConfig &rhs) {
    _name = rhs._name;
    _methode = rhs._methode;
    _port = rhs._port;
    _maxClient = rhs._maxClient;
    _location = rhs._location;
    _errorpages = rhs._errorpages;
    return (*this);
}

void    ServConfig::wrongFormatError(const char *msg, const char *line) {
    std::cerr << msg << " at this line: " << line << std::endl;
    throw ServConfig::wrongFormat();
}

void    ServConfig::initializeVariable(std::vector<std::string> tokens, std::string line, std::ifstream *confFile) {  
    (void) line;
    std::vector<std::string>::iterator it = tokens.begin();
    if (it->find("errorpages") != std::string::npos) {
        if (tokens.size() != 3)
            wrongFormatError("errorpages", "");
        _errorpages[std::atoi(tokens[1].c_str())] = tokens[2];
    }
    else if (it->find("server_names") != std::string::npos) {
        if (tokens.size() != 2)
            wrongFormatError("server_names", "");
        _name = tokens[1];
    }
    else if (tokens[0].find("listen")  != std::string::npos) {
        if (tokens.size() != 2)
            wrongFormatError("listen", "");
        _port = std::atoi(tokens[1].c_str());
    }
    else if (tokens[0].find("methode")  != std::string::npos) {
        if (tokens.size() < 2)
            wrongFormatError("methode", "");
        _methode = tokens[1];
    }
    else if (tokens[0].find("client_size")  != std::string::npos) {
        if (tokens.size() != 2)
            wrongFormatError("client_size", "");
        _maxClient = std::atoi(tokens[1].c_str());
    }
    else if (tokens[0].find("location")  != std::string::npos) {
        // std::cout << "OOOOOOEEEEE" << std::endl;
        if (tokens.size() != 3)
            wrongFormatError("location", "");
        Location Location;
        Location.init(tokens, confFile);
        _location.push_back(Location);
        // std::cout << Location;
        // exit(0);
    }
}

// void    ServConfig::initializeVariable(std::vector<std::string> tokens, std::ifstream *confFile) {
//     (void)  tokens;
//     (void)  confFile;
//     std::string tabFonction[6] = {"errorpages", "server_names", "listen", "methode", "client_size", "Location"};
//     std::vector<std::string> variables = {"errorpages", "server_names", "listen", "methode", "client_size", "Location"};
//     switch (0)
//     {
//     case 0:
//         /* code */
//         break;
    
//     default:
//         break;
//     }
// }  


void    ServConfig::initializeConfig(std::ifstream *confFile) {
    std::string line;
    bool        inServ = false;
    std::vector<std::string> tokens;
    
    while (std::getline(*confFile, line)) {
        std::stringstream SplitedLine(line);
        while (SplitedLine >> line) {
            tokens.push_back(line);
        }
        if (*tokens.begin() == "server") {
            if (!inServ)
                inServ = true;
            else
                throw MultipleServerOpen();
        }
        if (*tokens.begin() == "}") {
            // std::cout << std::endl << "-----------End of config server---------------" << std::endl << std::endl;
            return;
        }
        initializeVariable(tokens, line, confFile);
        // displayVector(tokens);
        tokens.clear();
    }
}

std::string ServConfig::pathToErrorPage(int pageToFind) {
    std::map<int, std::string>::iterator it = _errorpages.find(pageToFind);
    if (it != _errorpages.end()) {
        return (it->second);
    } else {
        return (NULL);
    }
}

//// Operator ////

std::ostream& operator<<(std::ostream& os, const ServConfig& obj) {
    os << MB "------" GREEN " Serveur Description " MB "------" C << std::endl;
    os << "Name: " << obj.getName() << std::endl;
    os << "Methode: " << obj.getMethode() << std::endl;
    os << "Port: " << obj.getPort() << std::endl;
    os << "MaxClient: " << obj.getMaxClient() << std::endl;
    os << std::endl << MB "--" C " Start " RED "Error" C " Pages " MB "--" C << std::endl;
    std::map<int, std::string> errorPages = obj.getErrorPages();
    for (std::map<int, std::string>::iterator it = errorPages.begin(); it != errorPages.end(); it++) {
        os << "error " << it->first << " = " << it->second << std::endl;
    }
    os << std::endl;
    std::vector<Location> location = obj.getLocation();
    for (std::vector<Location>::iterator it = location.begin(); it != location.end(); it++) {
        os << *it;
    }
    os << MB "------" RED "   End   Description " MB "------" C << std::endl;
    return (os);
}

//// Getter ////

std::string ServConfig::getName(void) const {
    return (this->_name);
}

std::string ServConfig::getMethode(void) const {
    return (this->_methode);
}

int         ServConfig::getPort(void) const {
    return (this->_port);
}

int         ServConfig::getMaxClient(void) const {
    return (this->_maxClient);
}

std::vector<Location>   ServConfig::getLocation(void) const {
    return (this->_location);
}

std::map<int, std::string> ServConfig::getErrorPages(void) const {
    return (this->_errorpages);
}

//// Throw ////

const char* ServConfig::wrongFormat::what(void) const throw() {
	return ("Config file: not properly formated");
}

const char* ServConfig::MultipleServerOpen::what(void) const throw() {
	return ("Config file: Multiple servers declared at the same time");
}
