/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gduchesn <gduchesn@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/25 15:17:47 by gduchesn          #+#    #+#             */
/*   Updated: 2024/03/25 15:17:48 by gduchesn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Location.hpp"

Location::Location() : _name(""), _methode(""), _port(0), _maxClient(0) {}

Location::Location(const Location &src) {*this = src;}

Location::~Location() {}

Location& Location::operator=(const Location &rhs) {
    _name = rhs._name;
    _methode = rhs._methode;
    _port = rhs._port;
    _maxClient = rhs._maxClient;
    _errorpages = rhs._errorpages;
    return (*this);
}
