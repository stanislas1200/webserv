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
    _templatePath = src._templatePath;
}

Location::~Location() {}

Location& Location::operator=(const Location &rhs) {
    _path = rhs._path;
    _methode = rhs._methode;
    _redirection = rhs._redirection;
    _pathToCgi = rhs._pathToCgi;
    _exCgi = rhs._exCgi;
    _templatePath = rhs._templatePath;
    return (*this);
}

std::vector<std::string>    Location::fillVectorInitialisation(void) {
    std::vector<std::string> vec;
    
    vec.push_back("methodes");
    vec.push_back("redirection");
    vec.push_back("cgi_path");
    vec.push_back("cgi_extention");
    vec.push_back("template");
    return (vec);
}

void Location::init(std::vector<std::string> tokens, std::ifstream *confFile) {
    std::vector<std::string> keyStack;
    std::string line;
    
    keyStack = fillVectorInitialisation();
    _path = tokens[1];
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
                _methode = line;
                if (tokens.size() < 2)
                    ServConfig::wrongFormatError("Location: methode", "need at least one methode autorised");
                _methode = vecToString(tokens.begin() + 1, tokens.end());
                break;
            case REDIRECTION:
                if (tokens.size() != 2)
                    ServConfig::wrongFormatError("Location: redirection", ERROR_HAPPEND);
                _redirection = tokens[1];
                break;
            case PATHTOCGI:
                if (tokens.size() != 2)
                    ServConfig::wrongFormatError("Location: pathToCgi", ERROR_HAPPEND);
                _pathToCgi = tokens[1];
                break;
            case EXCGI:
                if (tokens.size() < 2)
                    ServConfig::wrongFormatError("Location: exCgi", ERROR_HAPPEND);
                _exCgi = vecToString(tokens.begin() + 1, tokens.end());
                break;
            case LOCATION_TEMPLATE:
                if (tokens.size() != 2)
                    ServConfig::wrongFormatError("Location: template", ERROR_HAPPEND);
                _templatePath = tokens[1];
                break;
            default:
                    if (tokens[0] == "}")
                        return;
                    ServConfig::wrongFormatError("Location: Incoherent line:", ("\"" + vecToString(tokens.begin(), tokens.end()) + "\"").c_str());
        }
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

std::string Location::getTemplate(void) const {
    return (this->_templatePath);
}

//// Setter ////

void    Location::setMethode(std::string str) {
    _methode = str;
}

void    Location::setTemplate(std::string str) {
    _templatePath = str;
}

std::ostream& operator<<(std::ostream& os, const Location& obj) {
    os << "----Location----" << std::endl;
    
    os << "Path        : " << obj.getPath() << std::endl;
    os << "Methode     : " << obj.getMethode() << std::endl;
    os << "Redirection : " << obj.getRedirection() << std::endl;
    os << "PathToCgi   : " << obj.getPathToCgi() << std::endl;
    os << "ExCgi       : " << obj.getExCgi() << std::endl;
    os << "Template    : " << obj.getTemplate() << std::endl;
    
    os << "----End----" << std::endl;
    return (os);
}

bool            Location::operator==(const Location &rhs) {
    if (this->_path == rhs.getPath())
        return (true);
    return (false);
}

const char* Location::Error::what(void) const throw() {
	return ("error");
}