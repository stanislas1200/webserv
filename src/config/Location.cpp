/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gduchesn <gduchesn@students.s19.be>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/25 15:17:47 by gduchesn          #+#    #+#             */
/*   Updated: 2024/03/30 00:02:11 by gduchesn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Location.hpp"

Location::Location() {}

Location::Location(const Location &src) {
    _path = src._path;
    _methode = src._methode;
    _redirection = src._redirection;
    _pathToCgi = src._pathToCgi;
    _exCgi = src._exCgi;
}

Location::~Location() {}

Location& Location::operator=(const Location &rhs) {
    _path = rhs._path;
    _methode = rhs._methode;
    _redirection = rhs._redirection;
    _pathToCgi = rhs._pathToCgi;
    _exCgi = rhs._exCgi;
    return (*this);
}

void Location::init(std::vector<std::string> tokens, std::ifstream *confFile) {
    std::vector<std::string> keyStack = {"methodes", "redirection", "FUTURE PATH CGI", "FUTURE EXTENTION CGI", "template"};
    std::string line;
    
    if (tokens.size() >= 3) {
        _path = tokens[1];
        // std::cout << "PATH acquire" << std::endl;
    }
    tokens.clear();
    while (std::getline(*confFile, line)) {
        std::stringstream SplitedLine(line);
        while (SplitedLine >> line) {
            tokens.push_back(line);
        }
        if (tokens.empty())
            continue;
        line = vecToString(tokens.begin(), tokens.end());
        switch (getKey(keyStack, tokens[0])) {
            case LOCATION_METHODE:
                // std::cout << "METHODE acquire" << std::endl;
                _methode = line;
                break;
            case REDIRECTION:
                // std::cout << "REDIRECTION acquire" << std::endl;
                _redirection = *(++tokens.begin());
                break;
            case PATHTOCGI:
                // std::cout << "PATHTOCGI acquire" << std::endl;
                _pathToCgi = line;
                break;
            case EXCGI:
                // std::cout << "EXCGI acquire" << std::endl;
                _exCgi = line;
                break;
            case LOCATION_TEMPLATE:
                _templatePath = line;
                break;
            default:
                // if (tokens[0] == "}")
                    // std::cout << "location closed" << std::endl;
                break;
        }
        if (tokens[0] == "}")
            break;
        tokens.clear();
    }
}

//// Getter ////

std::string Location::getPath(void) const {
    return (this->_path);
}

std::string Location::getMethode(void) const {
    return (this->_methode);
}

std::string Location::getRedirection(void) const {
    return (this->_redirection);
}

std::string Location::getPathToCgi(void) const {
    return (this->_pathToCgi);
}

std::string Location::getExCgi(void) const {
    return (this->_exCgi);
}

std::ostream& operator<<(std::ostream& os, const Location& obj) {
    os << "----Location----" << std::endl;
    
    os << "Path        : " << obj.getPath() << std::endl;
    os << "Methode     : " << obj.getMethode() << std::endl;
    os << "Redirection : " << obj.getRedirection() << std::endl;
    os << "PathToCgi   : " << obj.getPathToCgi() << std::endl;
    os << "ExCgi       : " << obj.getExCgi() << std::endl;
    
    os << "----End----" << std::endl;
    return (os);
}

const char* Location::Error::what(void) const throw() {
	return ("error");
}