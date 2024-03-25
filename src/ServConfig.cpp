/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServConfig.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gduchesn <gduchesn@students.s19.be>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 14:43:18 by gduchesn          #+#    #+#             */
/*   Updated: 2024/03/25 01:32:45 by gduchesn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Webserv.hpp"

ServConfig::ServConfig() : _name(""), _methode(""), _port(0), _maxClient(0) {}

ServConfig::ServConfig(const ServConfig &src) {*this = src;}

ServConfig::~ServConfig() {}

ServConfig& ServConfig::operator=(const ServConfig &rhs) {
    if (&rhs != this) {
        return (*this);
    }
    return (*this);
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string::size_type start = 0;
    std::string::size_type end = str.find(delimiter);

    while (end != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }

    tokens.push_back(str.substr(start));

    return tokens;
}

void    ServConfig::wrongFormatError(const char *msg, const char *line) {
    std::cerr << msg << " at this line: " << line << std::endl;
    throw ServConfig::wrongFormat();
}

// void    ServConfig::initializeConfig(std::ifstream *confFile) {
//     std::string line;
    
//     int index = 0;
//     while (std::getline(*confFile, line) && ++index) {
//         std::cout << "" << line << std::endl;
//         std::stringstream ss;
//         std::stringstream SplitedLine(line);
//         while (!SplitedLine.fail()) {
            
//         }
//         ss << index;
//         // std::cout << "Servconfig.cpp " << line << std::endl;
//         if (line.find(";") == std::string::npos)
//             continue;
//         line.replace(line.find(";"), line.length() - line.find(";"), "");
//         std::vector<std::string> tokens = split(line, ' ');
//         if (tokens[0].find("server") != std::string::npos && index != 1) {
//             std::cout << "end because new serv" << std::endl; //
//             return;
//         }
//         // std::cout << tokens[0] << " " << tokens[0].find("errorpages") << std::endl;
//         if (tokens[0].find("errorpages") == 1) {
//             //need exeption when the setting file is not properly design !
//             if (tokens.size() != 3)
//                 wrongFormatError("errorpages", ss.str().c_str());
//             _errorpages[stoi(tokens[1])] = tokens[2];
//         }   else if (tokens[0].find("server_names") == 1) {
//             if (tokens.size() != 2)
//                 wrongFormatError("server_names", ss.str().c_str());
//             _name = tokens[1];
//         }   else if (tokens[0].find("listen") == 1) {
//             if (tokens.size() != 2)
//                 wrongFormatError("listen", ss.str().c_str());
//             _port = stoi(tokens[1]);
//         }   else if (tokens[0].find("methode") == 1) {
//             if (tokens.size() < 2)
//                 wrongFormatError("methode", ss.str().c_str());
//             _methode = line.erase(line.find("methode"), 8);
//         }   else if (tokens[0].find("client_size") == 1) {
//             if (tokens.size() != 2)
//                 wrongFormatError("client_size", ss.str().c_str());
//             _maxClient = stoi(tokens[1]);
//         }
//     }
//     std::cout << "Port: " << _port << std::endl;
//     //print the map
//     // for (std::map<int, std::string>::const_iterator it = _errorpages.begin(); it != _errorpages.end(); ++it) {
//     //     std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
//     // }
// }

void    ServConfig::initializeVariable(std::vector<std::string> tokens, std::string line) {  
    // std::cout << "Servconfig.cpp " << line << std::endl;
            // std::stringstream ss;
        // continue;
    // line.replace(line.find(";"), line.length() - line.find(";"), "");
    // std::vector<std::string> tokens = split(line, ' ');
    (void) line;
    if (tokens[0].find("server") != std::string::npos) {
        std::cout << "end because new serv" << std::endl; //
        return;
    }
    // std::cout << tokens[0] << " " << tokens[0].find("errorpages") << std::endl;
    if (tokens[0].find("errorpages") == 1) {
        //need exeption when the setting file is not properly design !
        if (tokens.size() != 3)
            wrongFormatError("errorpages", "");
        _errorpages[stoi(tokens[1])] = tokens[2];
    }   else if (tokens[0].find("server_names") == 1) {
        if (tokens.size() != 2)
            wrongFormatError("server_names", "");
        _name = tokens[1];
    }   else if (tokens[0].find("listen") == 1) {
        if (tokens.size() != 2)
            wrongFormatError("listen", "");
        _port = stoi(tokens[1]);
    }   else if (tokens[0].find("methode") == 1) {
        if (tokens.size() < 2)
            wrongFormatError("methode", "");
        // _methode = line.erase(line.find("methode"), 8);
    }   else if (tokens[0].find("client_size") == 1) {
        if (tokens.size() != 2)
            wrongFormatError("client_size", "");
        _maxClient = stoi(tokens[1]);
    }
}

void    ServConfig::initializeConfig(std::ifstream *confFile) {
    std::string line;
    bool        inServ = false;
    
    int index = 0;
    while (std::getline(*confFile, line)) {
        // std::cout << "" << line << std::endl;
        // std::stringstream ss;
        std::vector<std::string> tokens;
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
        if (*tokens.begin() == "}")
            return;
        // tokens.~vector();
        index++;
        initializeVariable(tokens, line);
        displayVector(tokens);
        tokens.clear();
        if (index == 2)
            break;
        // continue ;
        // ss << index;
    }
    // std::cout << "Port: " << _port << std::endl;
    //print the map
    // for (std::map<int, std::string>::const_iterator it = _errorpages.begin(); it != _errorpages.end(); ++it) {
    //     std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
    // }
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
    os << "Name: " << obj.getName() << std::endl;
    os << "Methode: " << obj.getMethode() << std::endl;
    os << "Port: " << obj.getPort() << std::endl;
    os << "MaxClient: " << obj.getMaxClient() << std::endl;
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
