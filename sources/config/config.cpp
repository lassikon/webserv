/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: janraub <janraub@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 15:38:39 by janraub           #+#    #+#             */
/*   Updated: 2024/08/21 10:48:59 by janraub          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config.hpp"

Config::Config(std::string configPath)
{
    _configFile.open(configPath);
    if (_configFile.fail())
    {
        std::cerr << "Error: Could not open config file" << std::endl;
        return;
    }
    std::stringstream configFileBuffer;
    configFileBuffer << _configFile.rdbuf();
    parseConfigFile(configFileBuffer);
}

Config::~Config()
{
    if (_configFile.is_open())
        _configFile.close();
}

Config::Config(Config const & src)
{
    *this = src;
}

Config & Config::operator=(Config const & src)
{
    if (this != &src)
    {
        _servers = src._servers;
    }
    return *this;
}

void Config::parseConfigFile(std::stringstream& configFile)
{    
    RouteConfig     routeTmp;
  
    while(std::getline(configFile, _line))
    {
        //remove comments, whitespaces and empty lines
        _line = Utility::trimCommentsAndWhitespaces(_line);
        if (_line.empty())
            continue;
        // find [server] block first
        if (_line.compare("[server]") == 0)
        {
            //init server struct and parse server block
            std::cout << "Found server block" << std::endl;
            parseServerBlock(configFile);
            // reread same line from parseServerBlock
            configFile.clear();
            configFile.seekg(_getlinePos);
        }
        else
        {
            //throw exception config error
            std::cout << "Error: Invalid header in config file" << std::endl;
            std::cout << _line << std::endl;
            return;
        }
    }
}

void Config::parseServerBlock(std::stringstream& configFile)
{
    ServerConfig    serverConfig;
    
    serverConfig = ServerConfig();
    while(std::getline(configFile, _line))
    {
        //remove comments, whitespaces and empty lines
        _line = Utility::trimCommentsAndWhitespaces(_line);
        if (_line.empty())
            continue;
        // find [route] block or populate server struct
        auto delimiter_pos = _line.find(":");
        if (delimiter_pos != std::string::npos)
        {
            std::cout << "Populating server block" << std::endl;
            std::string key = _line.substr(0, delimiter_pos);
            key = Utility::trimCommentsAndWhitespaces(key);
            std::string value = _line.substr(delimiter_pos + 1);
            value = Utility::trimCommentsAndWhitespaces(value);
            populateServer(serverConfig, key, value);
        }
        else if (_line.compare("[route]") == 0)
        {
            std::cout << "Found route block" << std::endl;
            //init route struct and parse route block
            parseRouteBlock(serverConfig, configFile);
            // reread same line from parseRouteBlock
            configFile.clear();
            configFile.seekg(_getlinePos);
        }
        else if (_line.compare("[server]") == 0)
            break;
        else
        {
            //throw exception config error
            std::cout << "Error: Invalid header in server block" << std::endl;
            return;
        }
        //get previous getline position
        _getlinePos = configFile.tellg();
    }
    //add server to map
    std::string hostName;
    if (serverConfig.serverName.empty())
        hostName = serverConfig.ipAddress + ":" + std::to_string(serverConfig.port);
    else
        hostName = serverConfig.serverName;
    _servers[hostName] = serverConfig;
    std::cout << "Server added to map" << std::endl;
}

//populate route block
void Config::parseRouteBlock(ServerConfig& serverConfig, std::stringstream& configFile)
{
    RouteConfig     routeConfig;
    
    routeConfig = RouteConfig();
    while(std::getline(configFile, _line))
    {
        //remove comments and whitespaces and empty lines
        _line = Utility::trimCommentsAndWhitespaces(_line);
        if (_line.empty())
            continue;
        // populate route struct
        auto delimiter_pos = _line.find(":");
        if (delimiter_pos != std::string::npos)
        {
            std::cout << "populating route block" << std::endl;
            std::string key = _line.substr(0, delimiter_pos);
            key = Utility::trimCommentsAndWhitespaces(key);
            std::string value = _line.substr(delimiter_pos + 1);
            value = Utility::trimCommentsAndWhitespaces(value);
            populateRoute(routeConfig, key, value);
        }
        else if(_line.compare("[route]") == 0 || _line.compare("[server]") == 0)
            break;
        else
        {
            //throw exception config error
            std::cout << "Error: Invalid header in route block" << std::endl;
            std::cout << _line << std::endl;
            return;
        }
        //get previous getline position
        _getlinePos = configFile.tellg();
    }
    //add route to server
    std::cout << "Route added to server" << std::endl;
    serverConfig.routes.push_back(routeConfig);
}
// populate server struct
void Config::populateServer(ServerConfig& serverConfig, std::string const & key, std::string const & value)
{
    static std::unordered_map<std::string, std::function<void(ServerConfig&, std::string const &)>> serverStructMap = {
        {"server_ip", setIP},
        {"server_name", setServerName},
        {"server_port", setPort},
        {"error_page", setErrorPages},
        {"client_body_size_limit", setClientBodySizeLimit},
    };
    auto it_key = serverStructMap.find(key);
    if (it_key != serverStructMap.end())
        it_key->second(serverConfig, value);
    else
    {
        //throw exception config error
        std::cout << "Error: Invalid key in server block" << std::endl;
        std::cout << _line << std::endl;
        return;
    }
        
}

// populate route struct
void Config::populateRoute(RouteConfig& routeConfig, std::string const & key, std::string const & value)
{
    static std::unordered_map<std::string, std::function<void(RouteConfig&, std::string const &)>> routeStructMap = {
        {"location", setLocation},
        {"methods", setMethods},
        {"root", setRoot},
        {"directory_listing", setDirectoryListing},
        {"default_file", setDefaultFile},
        {"upload_path", setUploadPath},
        {"redirect", setRedirect},
        {"cgi", setCgi}
    };
    auto it_key = routeStructMap.find(key);
    if (it_key != routeStructMap.end())
        it_key->second(routeConfig, value);
    else
    {
        //throw exception config error
        std::cout << "Error: Invalid key in route block" << std::endl;
        std::cout << _line << std::endl;
        return;
    }
}

// server struct setters
void Config::setIP(ServerConfig& server, std::string const & value)
{
    server.ipAddress = value;
}
void Config::setServerName(ServerConfig& server, std::string const & value)
{
    server.serverName = value;
}
void Config::setPort(ServerConfig& server, std::string const & value)
{
    server.port = std::stoi(value);
}
void Config::setErrorPages(ServerConfig& server, std::string const & value)
{
    std::stringstream ss(value);
    std::string error;
    std::getline(ss, error, ' ');
    int error_code = std::stoi(error);
    std::getline(ss, error, ' ');
    server.errorPages[error_code] = error;
}

void Config::setClientBodySizeLimit(ServerConfig& server, std::string const & value)
{
    server.clientBodySizeLimit = value;
}

// route struct setters

void Config::setLocation(RouteConfig& route, std::string const & value)
{
    route.location = value;
}

void Config::setMethods(RouteConfig& route, std::string const & value)
{
    std::stringstream ss(value);
    std::string method;
    while (std::getline(ss, method, ','))
    {
        method = Utility::trimCommentsAndWhitespaces(method);
        route.methods.push_back(method);
    }
}

void Config::setRoot(RouteConfig& route, std::string const & value)
{
    route.root = value;
}

void Config::setDirectoryListing(RouteConfig& route, std::string const & value)
{
    if (value == "on")
        route.directoryListing = true;
    else
        route.directoryListing = false;
}

void Config::setDefaultFile(RouteConfig& route, std::string const & value)
{
    route.defaultFile = value;
}

void Config::setUploadPath(RouteConfig& route, std::string const & value)
{
    route.uploadPath = value;
}

void Config::setRedirect(RouteConfig& route, std::string const & value)
{
    route.redirect = value;
}

void Config::setCgi(RouteConfig& route, std::string const & value)
{
    std::stringstream ss(value);
    std::string cgi;
    while (std::getline(ss, cgi, ','))
    {
        cgi = Utility::trimCommentsAndWhitespaces(cgi);
        route.cgi.push_back(cgi);
    }
}

void Config::printServerConfig()
{
    std::cout << "Printing server config" << std::endl;
    int serverCount = 1;
    for (const auto& server_pair : _servers)
    {
        const auto& server = server_pair.second;
        std::cout << "#Server: " << serverCount << std::endl;
        std::cout << "Server host: " << server.ipAddress << std::endl;
        std::cout << "Server port: " << server.port << std::endl;
        std::cout << "Server name: " << server.serverName << std::endl;
        std::cout << "Server error pages: ";
        for (const auto& error : server.errorPages)
        {
            std::cout << std::endl << "  " << error.first << "  path: " << error.second;
        }
        std::cout << std::endl;
        std::cout << "Server client body size limit: " << server.clientBodySizeLimit << std::endl;
        int routeCount = 1;
        for (const auto& route : server.routes)
        {
            std::cout << "#Server: " << serverCount << " route: " << routeCount << std::endl;
            std::cout << "  Route location: " << route.location << std::endl;
            std::cout << "  Route methods: ";
            for (const auto& method : route.methods)
            {
                std::cout << method << " ";
            }
            std::cout << std::endl;
            std::cout << "  Route cgi: ";
            for (const auto& cgi : route.cgi)
            {
                std::cout << cgi << " ";
            }
            std::cout << std::endl;
            std::cout << "  Route root: " << route.root << std::endl;
            std::cout << "  Route directory listing: " << route.directoryListing << std::endl;
            std::cout << "  Route default file: " << route.defaultFile << std::endl;
            std::cout << "  Route upload path: " << route.uploadPath << std::endl;
            std::cout << "  Route redirect: " << route.redirect << std::endl;
            routeCount++;
        }
        serverCount++;
    }
}