/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServConfig.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gduchesn <gduchesn@students.s19.be>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 14:43:18 by gduchesn          #+#    #+#             */
/*   Updated: 2024/06/13 02:13:14 by gduchesn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Webserv.hpp"
#include <cstring> // src/config/ServConfig.cpp:298:15: error: ‘strlen’ is not a member of ‘std’; did you mean ‘strlen’?

ServConfig::ServConfig() : _methode(""), _port(-1), _maxClient(-1), _autoindex(-1), _timeoutCgi(-1), _maxBody(-1) {}

ServConfig::~ServConfig() {}

ServConfig::ServConfig(const ServConfig &src) {
    _name = src._name;
    _methode = src._methode;
    _port = src._port;
    _maxClient = src._maxClient;
    _location = src._location;
    _errorpages = src._errorpages;
    _templatePath = src._templatePath;
    _autoindex = src._autoindex;
    _timeoutCgi = src._timeoutCgi;
    _root = src._root;
    _fd = src._fd;
    _sockaddr = src._sockaddr;
    _maxBody = src._maxBody;
}

ServConfig& ServConfig::operator=(const ServConfig &rhs) {
    _name = rhs._name;
    _methode = rhs._methode;
    _port = rhs._port;
    _maxClient = rhs._maxClient;
    _location = rhs._location;
    _errorpages = rhs._errorpages;
    _templatePath = rhs._templatePath;
    _autoindex = rhs._autoindex;
    _timeoutCgi = rhs._timeoutCgi;
    _root = rhs._root;
    _fd = rhs._fd;
    _sockaddr = rhs._sockaddr;
    _maxBody = rhs._maxBody;
    return (*this);
}

void    ServConfig::wrongFormatError(const char *msg, const char *line) {
    std::cerr << msg << " " << line << std::endl;
    throw wrongFormat();
}

std::vector<std::string>    ServConfig::fillVectorInitialisation(void) {
    std::vector<std::string> vec;

    vec.push_back("methodes");
    vec.push_back("errorpages");
    vec.push_back("listen");
    vec.push_back("server_names");
    vec.push_back("client_size");
    vec.push_back("location");
    vec.push_back("template");
    vec.push_back("autoindex");
    vec.push_back("timeoutCgi");
    vec.push_back("root");
    vec.push_back("max_body");
    return (vec);
}

void    ServConfig::initializeVariable(std::vector<std::string> tokens, std::ifstream *confFile) {
    int         result = 0;
    Location    location;
    std::vector<std::string> keyStack;
    
    keyStack = fillVectorInitialisation();
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
            if (tokens.size() != 3 || tokens[2] != "{")
                wrongFormatError("location", NOT_RIGHT);
            location.init(tokens, confFile);
            for (std::vector<Location>::iterator it = _location.begin(); it != _location.end(); it++) {
                if (_location.empty())
                    break;
                if (location == *it) {
                    _location.erase(it);
                    it = _location.begin();
                }
            }
            _location.push_back(location);
            break;
        case TEMPLATE:
            if (tokens.size() != 2)
                wrongFormatError("Temlate path", NOT_RIGHT);
            _templatePath = tokens[1];
            break;
        case AUTOINDEX:
            if (tokens.size() != 2)
                wrongFormatError("Autoindex", NOT_RIGHT);
            if (tokens[1] == "on")
                _autoindex = 1;
            else if (tokens[1] == "off")
                _autoindex = 0;
            break;
        case TIMEOUTCGI:
            if (tokens.size() != 2 || !isNbrNoOverflow(tokens[1], &result))
                wrongFormatError("Timeout cgi", NOT_RIGHT);
            _timeoutCgi = result;
            break;
        case ROOT:
            if (tokens.size() != 2)
                wrongFormatError("root", NOT_RIGHT);
            _root = tokens[1];
            break;
        case MAXBODY:
            if (tokens.size() != 2 || !isNbrNoOverflow(tokens[1], &result))
                wrongFormatError("max body", NOT_RIGHT);
            _maxBody = result;
            break;
        default:
            wrongFormatError("Incoherent line:", ("\"" + vecToString(tokens.begin(), tokens.end()) + "\"").c_str());
            break;
    }
}  

void    ServConfig::checkUpConfig(void) {
    if (_methode.empty())
        wrongFormatError("methode", MISSING);
    if (_port == -1)
        wrongFormatError("port", MISSING);
    if (_maxClient == -1)
        wrongFormatError("max client", MISSING);
    if (_location.empty())
        wrongFormatError("location", MISSING);
    if (_errorpages.empty())
        wrongFormatError("error pages", MISSING);
    if (_autoindex == -1)\
        wrongFormatError("Autoindex", MISSING);
    if (_timeoutCgi == -1)
        wrongFormatError("Timeout cgi", MISSING);
    if (_root.empty())
        wrongFormatError("root", MISSING);
    if (_maxBody == -1)
        wrongFormatError("max body", MISSING);
    for (std::vector<Location>::iterator it = _location.begin(); it != _location.end(); it++) {
        if (it->getPath().empty())
            wrongFormatError("Location: path", MISSING);
        if (it->getRedirection().empty())
            wrongFormatError("Location: redirection", MISSING);
        if (it->getRoot().empty())
            wrongFormatError("Location: root", MISSING);
        it->setRedirection(it->getRoot() + it->getRedirection());
        if (it->getMethode().empty())
            it->setMethode(_methode);
        if (it->getTemplate().empty() && !_templatePath.empty())
            it->setTemplate(_templatePath);
        if (it->getAutoindex() == -1)
            it->setAutoindex(_autoindex);
    }
}

bool    ServConfig::initializeConfig(std::ifstream *confFile) {
    std::string line;
    bool        inServ, started = false;
    std::vector<std::string> tokens;
    
    while (std::getline(*confFile, line)) {
        std::stringstream SplitedLine(line);
        while (SplitedLine >> line) {
            tokens.push_back(line);
        }
        if (tokens.empty()) {
            continue;
        }
        if (*tokens.begin() == "server") {
            if (!inServ) {
                inServ = started = true;
                tokens.clear();
                continue;
            }
            else
                throw MultipleServerOpen();
        }
        if (*tokens.begin() == "}") {
            inServ = false;
            break;
        }
        initializeVariable(tokens, confFile);
        tokens.clear();
    }
    if (!started)
        return (false);
    if (inServ)
        wrongFormatError("Server:", "bracket open");
    checkUpConfig();
    return (true);
}

std::string ServConfig::pathToErrorPage(int pageToFind) {
    std::map<int, std::string>::iterator it = _errorpages.find(pageToFind);
    if (it != _errorpages.end()) {
        return (it->second);
    } else {
        return (std::string());
    }
}

//// Operator ////

std::ostream& operator<<(std::ostream& os, const ServConfig& obj) {
    os << MB "------" GREEN " Serveur Description " MB "------" C << std::endl;
    os << "Name     : " << obj.getName() << std::endl;
    os << "Methode  : " << obj.getMethode() << std::endl;
    os << "Port     : " << obj.getPort() << std::endl;
    os << "MaxClient: " << obj.getMaxClient() << std::endl;
    os << "Template : " << obj.getTemplate() << std::endl;
    os << "Autoindex: " << obj.getAutoindex() << std::endl;
    os << "Timeout  : " << obj.getTimeoutCgi() << std::endl;
    os << "maxBody  : " << obj.getMaxBody() << std::endl;
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

std::string ServConfig::getTemplate(void) const {
    return (this->_templatePath);
}

int ServConfig::getAutoindex(void) const {
    return (this->_autoindex);
}

int ServConfig::getTimeoutCgi(void) const {
    return (this->_timeoutCgi);
}

std::string ServConfig::getRoot(void) const {
    return (this->_root);
}

int ServConfig::getFd(void) const {
    return (this->_fd);
}

int ServConfig::getMaxBody(void) const {
    return (this->_maxBody);
}

void ServConfig::setFd(int fd) {
    this->_fd = fd;
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

bool    isNbrNoOverflow(std::string token, int *result) {
    for (size_t i = 0; i < token.size(); i++) {
        if (!std::isdigit(token[i])) {
            return (false);
        }
    }
    if ((std::strlen(token.c_str()) >= 10 && std::strcmp(token.c_str(), "2147483647") > 0) || std::strlen(token.c_str()) > 10)
        return (false);
    *result = std::atoi(token.c_str());
    return (true);
}
