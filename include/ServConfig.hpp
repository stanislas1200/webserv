/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServConfig.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gduchesn <gduchesn@students.s19.be>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 14:33:10 by gduchesn          #+#    #+#             */
/*   Updated: 2024/02/16 16:28:48 by gduchesn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Webserv.hpp"

class ServConfig {
    private:
        std::string _name;
        std::string _methode;
        int         _port;
        int         _maxClient;
        // std::vector<std::string>    methode;
        std::map<int, std::string> _errorpages;
        void    wrongFormatError(const char *msg, const char *line);
    public:
        ServConfig(void);
        ServConfig(const ServConfig &src);
        ~ServConfig(void);
        ServConfig& operator=(const ServConfig &rhs);
        
        void        initializeConfig(std::ifstream *confFile);
        std::string pathToErrorPage(int pageToFind);
        
        std::string getName(void);
        std::string getMethode(void);
        int         getPort(void);
        int         getMaxClient(void);
        std::map<int, std::string> getErrorPages(void);
        
		class wrongFormat : public std::exception {
			public :
				virtual const char* what(void) const throw();
		};
};
