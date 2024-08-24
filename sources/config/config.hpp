/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: janraub <janraub@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 15:38:16 by janraub           #+#    #+#             */
/*   Updated: 2024/08/24 08:11:17 by janraub          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <unordered_map>
#include <functional>
#include <regex>
#include <filesystem>

#include "utility.hpp"
#include <Logger.hpp>

struct RouteConfig
{
    std::string                 location;
    std::vector<std::string>    methods;
    std::string                 root;
    bool                        directoryListing;
    std::string                 defaultFile;
    std::string                 uploadPath;
    std::string                 redirect;
	std::vector<std::string>    cgi;
};

struct ServerConfig
{
    std::string                     ipAddress;
    std::string                     serverName;
    int                             port;
	std::map<int, std::string>      errorPagesInternal = {{200, "/pages/200.html"}, // ok
									{400, "/pages/400.html"}, // bad request
									{413, "/pages/413.html"}, // request entity too large
									{413, "/pages/413.html"}, // request entity too large
									{404, "/pages/404.html"}, // not found
									{405, "/pages/405.html"}, // method not allowed
									{500, "/pages/500.html"}, // internal server error
									{501, "/pages/501.html"}}; // not implemented
									
    std::map<int, std::string>      errorPages;
    std::string                     clientBodySizeLimit;
    std::vector<RouteConfig>        routes;
};

class Config
{
    private:
		std::map<std::string, ServerConfig> _servers;
		std::ifstream						_configFile;
		std::string     					_line;
		std::stack<std::string>				_blockStack;
        std::streampos                      _pos;
		static int							_lineNumber;
    public:
        Config(std::string configPath);
        ~Config();
        Config(Config const & src);
        Config & operator=(Config const & src);

        void  parseConfigFile(std::stringstream& configFile);
        void  parseServerBlock(std::stringstream& configFile);
		    void  populateServer(ServerConfig& serverConfig, std::size_t & pos);
        void  parseRouteBlock(ServerConfig& serverConfig ,std::stringstream& configFile);
		    void	populateRoute(RouteConfig& routeConfig, std::size_t & pos);
		    void	addServerToMap(ServerConfig& serverConfig);

        // helper function
        bool	callGetLine(std::stringstream& configFile);
        
        // server struct setters
        static void	setIP(ServerConfig& server, std::string const & value);
        static void	setServerName(ServerConfig& server, std::string const & value);
        static void	setPort(ServerConfig& server, std::string const & value);
        static void	setErrorPages(ServerConfig& server, std::string const & value);
        static void	setClientBodySizeLimit(ServerConfig& server, std::string const & value);

        // route struct setters
        static void	setLocation(RouteConfig& route, std::string const & value);
        static void	setMethods(RouteConfig& route, std::string const & value);
        static void	setRoot(RouteConfig& route, std::string const & value);
        static void	setDirectoryListing(RouteConfig& route, std::string const & value);
        static void	setDefaultFile(RouteConfig& route, std::string const & value);
        static void	setUploadPath(RouteConfig& route, std::string const & value);
        static void	setRedirect(RouteConfig& route, std::string const & value);
        static void	setCgi(RouteConfig& route, std::string const & value);
        
        // print server config
        void printServerConfig();

        // get server config
        int getLineNumber() const;
		
        std::vector<ServerConfig> const & getServers() const;
};