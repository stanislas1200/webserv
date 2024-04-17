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
# define METHODE        0
# define ERRORPAGES     1
# define LISTEN         2
# define SERVER_NAMES   3
# define CLIENT_SIZE    4
# define LOCATION       5
# define TEMPLATE       6
# define NOT_RIGHT "not right amount of parameter"
# define ERROR_HAPPEND "something wrong in parameters"
# define MISSING "mandatory information missing"

class Location;

class ServConfig {
    private:
        std::string                 _name;
        std::string                 _methode;
        int                         _port;
        int                         _maxClient;
        std::vector<Location>       _location;
        std::map<int, std::string>  _errorpages;
        std::string                 _templatePath;
        void    initializeVariable(std::vector<std::string> tokens, std::ifstream *confFile);
        std::vector<std::string>    fillVectorInitialisation(void);

    public:
        ServConfig(void);
        ServConfig(const ServConfig &src);
        virtual ~ServConfig(void);
        ServConfig& operator=(const ServConfig &rhs);
        
        static void wrongFormatError(const char *msg, const char *line);
        void        initializeConfig(std::ifstream *confFile);
        std::string pathToErrorPage(int pageToFind);
        void        checkUpConfig(void);
        
        std::string getName(void) const;
        std::string getMethode(void) const;
        int         getPort(void) const;
        int         getMaxClient(void) const;
        std::string getTemplate(void) const;
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
        class Methode : public std::exception {
			public :
				virtual const char* what(void) const throw();
		};
};

class OverflowNbr : public std::exception {
    public :
        virtual const char* what(void) const throw();
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
int     getKey(std::vector<std::string> key, std::string token);
std::string vecToString(std::vector<std::string>::iterator begin, std::vector<std::string>::iterator end);
bool    isNbrNoOverflow(std::string token, int *result);