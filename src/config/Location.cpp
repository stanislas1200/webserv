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

Location::Location(const Location &src) {*this = src;}

Location::~Location() {}

Location& Location::operator=(const Location &rhs) {
    (void) rhs;
    return (*this);
}

void Location::init(std::vector<std::string> tokens, std::ifstream *confFile) {
    std::string line;
    
    while (std::getline(*confFile, line)) {
        
    }
}

std::ostream& operator<<(std::ostream& os, const Location& obj) {
    (void) obj;
    os << "----Location----" << std::endl;
    
    os << std::endl;
    
    os << "----End----" << std::endl;
    return (os);
}

const char* Location::Error::what(void) const throw() {
	return ("error");
}