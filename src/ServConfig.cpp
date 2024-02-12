/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServConfig.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gduchesn <gduchesn@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 14:43:18 by gduchesn          #+#    #+#             */
/*   Updated: 2024/02/12 14:43:20 by gduchesn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

ServConfig::ServConfig() {};

ServConfig::ServConfig(const ServConfig &src) {*this = src;};

ServConfig::~ServConfig() {};

ServConfig& ServConfig::operator=(const ServConfig)

