/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gduchesn <gduchesn@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/25 15:17:10 by gduchesn          #+#    #+#             */
/*   Updated: 2024/03/25 15:17:11 by gduchesn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Webserv.hpp"

class Location {
    private:
        std::string 
    public:
        Location(void);
        Location(const Location &src);
        ~Location(void);
        Location& operator=(const Location &rhs);
        
		class wrongFormat : public std::exception {
			public :
				virtual const char* what(void) const throw();
		};
};

std::ostream& operator<<(std::ostream& os, const Location& obj);
