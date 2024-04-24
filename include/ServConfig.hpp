/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServConfig.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sgodin <sgodin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 14:33:10 by gduchesn          #+#    #+#             */
/*   Updated: 2024/04/24 15:06:28 by sgodin           ###   ########.fr       */
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
# define AUTOINDEX      7
# define TIMEOUTCGI     8
# define NOT_RIGHT "not right amount of parameter"
# define ERROR_HAPPEND "something wrong in parameters"
# define MISSING "mandatory information missing"

class Location;

class ServConfig {
    private:
        std::string                 _name; // nop
        std::string                 _methode;
        int                         _port;
        int                         _maxClient;
        std::vector<Location>       _location;
        std::map<int, std::string>  _errorpages;
        std::string                 _templatePath; // nop
        bool                        _autoindex;
        int                         _timeoutCgi;
	    int                         _fd;
        
        void    initializeVariable(std::vector<std::string> tokens, std::ifstream *confFile);
        std::vector<std::string>    fillVectorInitialisation(void);

    public:
        sockaddr_in _sockaddr;
        ServConfig(void);
        ServConfig(const ServConfig &src);
        virtual ~ServConfig(void);
        ServConfig& operator=(const ServConfig &rhs);
        
        static void wrongFormatError(const char *msg, const char *line);
        bool        initializeConfig(std::ifstream *confFile);
        std::string pathToErrorPage(int pageToFind);
        void        checkUpConfig(void);
        
        /// getter ///
        std::string getName(void) const;
        std::string getMethode(void) const;
        int         getPort(void) const;
        int         getMaxClient(void) const;        
        std::string getTemplate(void) const;
        std::vector<Location>   getLocation(void) const;
        std::map<int, std::string> getErrorPages(void) const;
        bool        getAutoindex(void) const;
        int         getTimeoutCgi(void) const;        
        int         getFd(void) const;
        void         setFd(int fd);
        
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

# define LOCATION_METHODE   0
# define REDIRECTION        1
# define ROOT               2
# define PATHTOCGI          3
# define EXCGI              4
# define LOCATION_TEMPLATE  5
# define LOCATION_AUTOINDEX 6

class Location {
    private:
        std::string _path;
        std::string _methode; // nop
        std::string _redirection;
        std::string _root;
        std::string _pathToCgi; //nop
        std::string _exCgi; // nop
        std::string _templatePath; // nop
        bool        _autoindex; //nop
        std::vector<std::string>    fillVectorInitialisation(void);
    public:
        Location(void);
        Location(const Location &src);
        virtual ~Location(void);
        Location&   operator=(const Location &rhs);
        bool        operator==(const Location &rhs);

        void init(std::vector<std::string> tokens, std::ifstream *confFile);
        
        //////  getter //////

        std::string getPath(void) const;
        std::string getMethode(void) const;
        std::string getRedirection(void) const;
        std::string getRoot(void) const;
        std::string getPathToCgi(void) const;
        std::string getExCgi(void) const;
        std::string getTemplate(void) const;
        bool        getAutoindex(void) const;
        
        //////  setter  //////

        void    setMethode(std::string str);
        void    setTemplate(std::string str);

        class Error : public std::exception {
			public :
				virtual const char* what(void) const throw();
		};
};

std::ostream&   operator<<(std::ostream& os, const Location& obj);

typedef struct s_file {
	std::string name;
	std::string	path;
}	t_file;

typedef struct s_config {
	int port;
}	t_config;

typedef struct s_FormDataPart {
    std::string header;
	std::string name;
	std::string filename;
	std::string contentType;
	std::vector<char> data;
	size_t bodySize;
	bool full;
} t_FormDataPart;

typedef struct s_request {
	std::string method;
	std::string path;
	std::map<std::string, std::string> headers;
	std::string body;
	std::string boundary;
    std::string queryString;
	size_t dataLen;
	struct s_FormDataPart formData[2];
	int connection;
	ServConfig conf;
	Location loc;
} t_request;

void error(const char *type, const char *msg, const char *bold);

struct s_request;

int parseRequest(std::string header, s_request *request);
void error(const char *type, const char *msg, const char *bold);
void acceptConnection(std::vector<ServConfig> config);
void	sendFile(int connection, std::ifstream *file, int status, s_request request);
void printRequest(s_request request);
std::string readHeader(int connection);
