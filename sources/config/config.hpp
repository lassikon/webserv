/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: janraub <janraub@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 15:38:16 by janraub           #+#    #+#             */
/*   Updated: 2024/08/21 10:44:39 by janraub          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include "utility.hpp"

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
		std::streampos						_getlinePos;
    public:
        Config(std::string configPath);
        ~Config();
        Config(Config const & src);
        Config & operator=(Config const & src);

        void    parseConfigFile(std::stringstream& configFile);
        void    parseServerBlock(std::stringstream& configFile);
		void	populateServer(ServerConfig& serverConfig, std::string const & key, std::string const & value);
        void    parseRouteBlock(ServerConfig& serverConfig ,std::stringstream& configFile);
		void	populateRoute(RouteConfig& routeConfig, std::string const & key, std::string const & value);
		// server struct setters
		static void	setIP(ServerConfig& server, std::string const & value);
		static void	setServerName(ServerConfig& server, std::string const & value);
		static void	setPort(ServerConfig& server, std::string const & value);
		static void	setErrorPages(ServerConfig& server, std::string const & value);
		static void	setClientBodySizeLimit(ServerConfig& server, std::string const & value);
		//static void	setRoutes(ServerConfig& server, std::string const & value);

		// route struct setters
		static void	setLocation(RouteConfig& route, std::string const & value);
		static void	setMethods(RouteConfig& route, std::string const & value);
		static void	setRoot(RouteConfig& route, std::string const & value);
		static void	setDirectoryListing(RouteConfig& route, std::string const & value);
		static void	setDefaultFile(RouteConfig& route, std::string const & value);
		static void	setUploadPath(RouteConfig& route, std::string const & value);
		static void	setRedirect(RouteConfig& route, std::string const & value);
		static void	setCgi(RouteConfig& route, std::string const & value);
		
		// getters

		// print server config
		void printServerConfig();
		
        std::vector<ServerConfig> const & getServers() const;
};