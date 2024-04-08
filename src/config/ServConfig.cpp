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
        if (tokens.size() != 3)
            wrongFormatError("location", "");
        Location Location;
        Location.init(tokens, confFile);
        _location.push_back(Location);
        // std::cout << Location;
        // exit(0);
    }
}

void    ServConfig::wrongFormatError(const char *msg, const char *line) {
    std::cerr << msg << " " << line << std::endl;
    throw wrongFormat();
}

bool    isNbrNoOverflow(std::string token, int *result) {
    for (size_t i = 0; i < token.size(); i++) {
        if (!std::isdigit(token[i]))
            return (false);
            // ServConfig::wrongFormatError(msg, "not a nbr in parameter");
    }
    if ((std::strlen(token.c_str()) >= 10 && std::strcmp(token.c_str(), "2147483647") > 0) || std::strlen(token.c_str()) > 10)
        return (false);
        // throw OverflowNbr();
    *result = std::atoi(token.c_str());
    return (true);
}

void    ServConfig::initializeVariable(std::vector<std::string> tokens, std::ifstream *confFile) {
    (void)  tokens;
    (void)  confFile;
    int         result = 0;
    Location    Location;
    std::vector<std::string> keyStack = {"methodes", "errorpages", "listen", "server_names", "client_size", "location", "template"};
    switch (getKey(keyStack, tokens[0]))
    {
        case METHODE:
            if (tokens.size() < 2)
                wrongFormatError("methode", "need at least one methode autorised");
            _methode = vecToString(tokens.begin() + 1, tokens.end());
            break;
        case ERRORPAGES:
            if (tokens.size() != 3 || !isNbrNoOverflow(tokens[1], &result))
                wrongFormatError("errorpages", ERROR_HAPPEND);
            _errorpages[result] = tokens[2];
            break;
        case LISTEN:
            if (tokens.size() != 2 || !isNbrNoOverflow(tokens[1], &result))
                wrongFormatError("listen", ERROR_HAPPEND);
            _port = result;
            break;
        case SERVER_NAMES:
            if (tokens.size() != 2)
                wrongFormatError("server_names", NOT_RIGHT);
            _name = tokens[1];
            break;
        case CLIENT_SIZE:
            if (tokens.size() != 2 || !isNbrNoOverflow(tokens[1], &result))
                wrongFormatError("client_size", ERROR_HAPPEND);
            _maxClient = result;
            break;
        case LOCATION:
            if (tokens.size() != 3)
                wrongFormatError("location", NOT_RIGHT);
            Location.init(tokens, confFile);
            _location.push_back(Location);
            break;
        case TEMPLATE:
            if (tokens.size() != 2)
                wrongFormatError("Temlate path", NOT_RIGHT);
            break;
        default:
            // std::cout << tokens[0] << std::endl;
            // if (!tokens.empty())
                wrongFormatError("Incoherent line:", ("\"" + vecToString(tokens.begin(), tokens.end()) + "\"").c_str());
            break;
    }
}  


void    ServConfig::initializeConfig(std::ifstream *confFile) {
    std::string line;
    bool        inServ = false;
    std::vector<std::string> tokens;
    
    while (std::getline(*confFile, line)) {
        std::stringstream SplitedLine(line);
        while (SplitedLine >> line) {
            tokens.push_back(line);
        }
        if (tokens.empty())
            continue;
        if (*tokens.begin() == "server") {
            if (!inServ) {
                inServ = true;
                tokens.clear();
                continue;
            }
            else
                throw MultipleServerOpen();
        }
        if (*tokens.begin() == "}") {
            // std::cout << std::endl << "-----------End of config server---------------" << std::endl << std::endl;
            return;
        }
        // initializeVariable(tokens, line, confFile);
        initializeVariable(tokens, confFile);
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

const char* OverflowNbr::what(void) const throw() {
	return ("Config file: Number overflow");
}

int getKey(std::vector<std::string> keyStack, std::string token) {
    int index = 0;
    
    for (std::vector<std::string>::iterator it = keyStack.begin(); it != keyStack.end(); ++it) {
        if (*it == token) {
            break;
        }
        index++;
    }
    return (index);
}

std::string vecToString(std::vector<std::string>::iterator begin, std::vector<std::string>::iterator end) {
    std::string str;
    
    for (std::vector<std::string>::iterator it = begin; it != end; ++it) {
        str += *it;
        if (it + 1 != end) {
            str += " ";
        }
    }
    return (str);
}