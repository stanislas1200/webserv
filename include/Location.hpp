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
# define METHODE        0
# define REDIRECTION    1
# define PATHTOCGI      2
# define EXCGI          3

class Location {
    private:
        std::string _path;
        std::string _methode;
        std::string _redirection;
        std::string _pathToCgi;
        std::string _exCgi;
    public:
        Location(void);
        Location(const Location &src);
        virtual ~Location(void);
        Location& operator=(const Location &rhs);

        void init(std::vector<std::string> tokens, std::ifstream *confFile);
        
        //////  getter //////

        std::string getPath(void) const;
        std::string getMethode(void) const;
        std::string getRedirection(void) const;
        std::string getPathToCgi(void) const;
        std::string getExCgi(void) const;

        class Error : public std::exception {
			public :
				virtual const char* what(void) const throw();
		};
};

std::ostream& operator<<(std::ostream& os, const Location& obj);
