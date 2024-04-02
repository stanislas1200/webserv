/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServConfig.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gduchesn <gduchesn@students.s19.be>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 14:33:10 by gduchesn          #+#    #+#             */
/*   Updated: 2024/03/30 00:09:38 by gduchesn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Webserv.hpp"

class Location;

class ServConfig {
    private:
        std::string                 _name;
        std::string                 _methode;
        int                         _port;
        int                         _maxClient;
        std::vector<Location>       _location;
        std::map<int, std::string>  _errorpages;
        void    wrongFormatError(const char *msg, const char *line);
        void    initializeVariable(std::vector<std::string> tokens, std::string line, std::ifstream *confFile);
        void    initializeVariable(std::vector<std::string> tokens, std::ifstream *confFile);
    public:
        ServConfig(void);
        ServConfig(const ServConfig &src);
        virtual ~ServConfig(void);
        ServConfig& operator=(const ServConfig &rhs);
        
        void        initializeConfig(std::ifstream *confFile);
        std::string pathToErrorPage(int pageToFind);
        
        std::string getName(void) const;
        std::string getMethode(void) const;
        int         getPort(void) const;
        int         getMaxClient(void) const;
        std::vector<Location>   getLocation(void) const;
        std::map<int, std::string> getErrorPages(void) const;
        
		class wrongFormat : public std::exception {
			public :
				virtual const char* what(void) const throw();
		};
		class MultipleServerOpen : public std::exception {
			public :
				virtual const char* what(void) const throw();
		};
};

template<typename T>
void displayVector(const std::vector<T>& vec) {
    typename std::vector<T>::const_iterator it;

    std::cout << "Contents of the vector:" << std::endl;
    for (it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << std::endl;
    }
}

std::ostream& operator<<(std::ostream& os, const ServConfig& obj);