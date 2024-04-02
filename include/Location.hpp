/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gduchesn <gduchesn@students.s19.be>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/25 15:17:10 by gduchesn          #+#    #+#             */
/*   Updated: 2024/03/29 23:58:49 by gduchesn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Webserv.hpp"

class Location : public ServConfig {
    private:
        std::string tmp; 
    public:
        Location(void);
        Location(const Location &src);
        ~Location(void);
        Location& operator=(const Location &rhs);

        void init(std::vector<std::string> tokens, std::ifstream *confFile);
        
		// class wrongFormat : public std::exception {
		// 	public :
		// 		virtual const char* what(void) const throw();
		// };
        class Error : public std::exception {
			public :
				virtual const char* what(void) const throw();
		};
};

std::ostream& operator<<(std::ostream& os, const Location& obj);
