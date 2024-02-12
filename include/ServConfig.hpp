/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServConfig.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gduchesn <gduchesn@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 14:33:10 by gduchesn          #+#    #+#             */
/*   Updated: 2024/02/12 14:33:16 by gduchesn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Webserv.hpp"

class ServConfig {
    public:
        ServConfig(void);
        ServConfig(const ServConfig &src);
        ~ServConfig(void);

    private:
	    std::string name;
	    std::string	path;
}	t_config;
