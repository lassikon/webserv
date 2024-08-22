/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: janraub <janraub@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 15:38:39 by janraub           #+#    #+#             */
/*   Updated: 2024/08/22 13:58:13 by janraub          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config.hpp"

Config::Config(std::string configPath)
{
    LOG_DEBUG("Config constructor");
    _configFile.open(configPath);
    if (_configFile.fail())
        throw std::runtime_error("Error: Could not open config file");
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
        _servers = src._servers;
    return *this;
}

void Config::parseConfigFile(std::stringstream& configFile)
{    
    while(callGetLine(configFile))
    {
        if (_line.empty())
            continue;
        if (_line.compare("[server]") == 0)
        {
            _blockStack.push("[server]");
            parseServerBlock(configFile);
            if (!_blockStack.empty() && _blockStack.top() == "[server]")
                throw std::runtime_error("Error: unclosed server block");
        }
        else
            throw std::runtime_error("Error: no server block header found, " + _line);
    }
}

void Config::parseServerBlock(std::stringstream& configFile)
{
    ServerConfig    serverConfig;
    
    serverConfig = ServerConfig{};
    while(callGetLine(configFile))
    {
        if (_line.empty())
            continue;
        auto delimiter_pos = _line.find(":");
        if (delimiter_pos != std::string::npos)
            populateServer(serverConfig, delimiter_pos);
        else if (_line.compare("[server]") == 0)
            return;
        else if (_line.compare("[route]") == 0) 
        {
            _blockStack.push("[route]");
            parseRouteBlock(serverConfig, configFile);
            if (!_blockStack.empty() && _blockStack.top() == "[route]")
                throw std::runtime_error("Error: unclosed route block");
        }
        else if (_line.compare("[/server]") == 0 && (!_blockStack.empty() && _blockStack.top() == "[server]"))
        {
            _blockStack.pop();
            addServerToMap(serverConfig);
            return;
        }
        else
            throw std::runtime_error("Error: Invalid line in server block, " + _line);
    }
}

//populate route block
void Config::parseRouteBlock(ServerConfig& serverConfig, std::stringstream& configFile)
{
    RouteConfig     routeConfig;
    
    routeConfig = RouteConfig{};
    while(callGetLine(configFile))
    {
        if (_line.empty())
            continue;
        auto delimiter_pos = _line.find(":");
        if (delimiter_pos != std::string::npos)
            populateRoute(routeConfig, delimiter_pos);
        else if (_line.compare("[route]") == 0)
            return;
        else if(_line.compare("[/route]") == 0 && (!_blockStack.empty() && _blockStack.top() == "[route]"))
        {
            _blockStack.pop();
            serverConfig.routes.push_back(routeConfig);
            return;
        }
        else
            throw std::runtime_error("Error: Invalid line in route block, " + _line);
    }
}
// populate server struct
void Config::populateServer(ServerConfig& serverConfig, std::size_t & pos)
{
    static std::unordered_map<std::string, std::function<void(ServerConfig&, std::string const &)>> serverStructMap = {
        {"server_ip", setIP},
        {"server_name", setServerName},
        {"server_port", setPort},
        {"error_page", setErrorPages},
        {"client_body_size_limit", setClientBodySizeLimit},
    };
    std::string key = _line.substr(0, pos);
    key = Utility::trimCommentsAndWhitespaces(key);
    std::string value = _line.substr(pos + 1);
    value = Utility::trimCommentsAndWhitespaces(value);
    auto it_key = serverStructMap.find(key);
    if (it_key != serverStructMap.end())
    {   
        if (value.empty())
            throw std::runtime_error("Error: Value not found for key " + key);
        it_key->second(serverConfig, value);
    }
    else
        throw std::runtime_error("Error: Invalid key in server block, " + key);
}

// populate route struct
void Config::populateRoute(RouteConfig& routeConfig, std::size_t & pos)
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
    std::string key = _line.substr(0, pos);
    key = Utility::trimCommentsAndWhitespaces(key);
    std::string value = _line.substr(pos + 1);
    value = Utility::trimCommentsAndWhitespaces(value);
    auto it_key = routeStructMap.find(key);
    if (it_key != routeStructMap.end())
    {
        if (value.empty())
            throw std::runtime_error("Error: Value not found for key " + key);
        it_key->second(routeConfig, value);
    }
    else
        throw std::runtime_error("Error: Invalid key in route block, " + key);
}
// NGINX uses ip:port first, server_name second and default server last
void Config::addServerToMap(ServerConfig& serverConfig)
{
    std::string hostName;
    if (serverConfig.port != 0)
        hostName = serverConfig.ipAddress + ":" + std::to_string(serverConfig.port);
    else
        hostName = serverConfig.serverName;
    if (_servers.find(hostName) != _servers.end())
        throw std::runtime_error("Error: Server already exists");
    _servers[hostName] = serverConfig;
}

// server struct setters
void Config::setIP(ServerConfig& server, std::string const & value)
{
    if (!server.ipAddress.empty())
        throw std::runtime_error("Error: IP already set");
    const std::regex ipPattern("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$");
    if (!std::regex_match(value, ipPattern))
        throw std::runtime_error("Error: Invalid IP address");
    server.ipAddress = value;
}
void Config::setServerName(ServerConfig& server, std::string const & value)
{
    if (!server.serverName.empty())
        throw std::runtime_error("Error: Server name already set");
 
    const std::regex serverNamePattern("^(\\w)[\\w-]{0,61}(\\w)(\\.[\\w-]{1,63})*$");
    if (!std::regex_match(value, serverNamePattern))
        throw std::runtime_error("Error: Invalid server name");
    server.serverName = value;
}
void Config::setPort(ServerConfig& server, std::string const & value)
{
    if (server.port != 0)
        throw std::runtime_error("Error: Port already set");
    const std::regex port_pattern("^[0-9]+$");
    if (!std::regex_match(value, port_pattern))
        throw std::runtime_error("Error: Invalid port number");
    if (std::stoi(value) < 0 || std::stoi(value) > 65535)
        throw std::runtime_error("Error: Port number out of range");
    server.port = std::stoi(value);
}
void Config::setErrorPages(ServerConfig& server, std::string const & value)
{
    if (value.empty())
        throw std::runtime_error("Error: Error code not found");
    std::stringstream ss(value);
    std::string error;
    std::getline(ss, error, ' ');
    const std::regex error_pattern("^[0-9]+$");
    if (!std::regex_match(error, error_pattern))
        throw std::runtime_error("Error: Invalid error code");
    int error_code = std::stoi(error);
    if (server.errorPages.find(error_code) != server.errorPages.end())
        throw std::runtime_error("Error: Error code already set");
    std::getline(ss, error, ' ');
    if (error.empty())
        throw std::runtime_error("Error: Error page path not found");
    //std::filesystem::path path(error);

    if (server.errorPagesInternal.find(error_code) != server.errorPagesInternal.end())
        server.errorPages[error_code] = error;
}
// client body size limit is set in bytes, kilobytes, megabytes
void Config::setClientBodySizeLimit(ServerConfig& server, std::string const & value)
{
    if (!server.clientBodySizeLimit.empty())
        throw std::runtime_error("Error: Client body size limit already set");
    server.clientBodySizeLimit = value;
}

// route struct setters

void Config::setLocation(RouteConfig& route, std::string const & value)
{
    if (!route.location.empty())
        throw std::runtime_error("Error: Location already set");
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
    if (!route.root.empty())
        throw std::runtime_error("Error: Root already set");
    route.root = value;
}

void Config::setDirectoryListing(RouteConfig& route, std::string const & value)
{
    if (value == "on")
        route.directoryListing = true;
    else if (value == "off")
        route.directoryListing = false;
    else
        throw std::runtime_error("Error: Invalid value for directory listing");
}

void Config::setDefaultFile(RouteConfig& route, std::string const & value)
{
    if (!route.defaultFile.empty())
        throw std::runtime_error("Error: Default file already set");
    route.defaultFile = value;
}

void Config::setUploadPath(RouteConfig& route, std::string const & value)
{
    if (!route.uploadPath.empty())
        throw std::runtime_error("Error: Upload path already set");
    route.uploadPath = value;
}

void Config::setRedirect(RouteConfig& route, std::string const & value)
{
    if (!route.redirect.empty())
        throw std::runtime_error("Error: Redirect already set");
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

bool Config::callGetLine(std::stringstream& configFile)
{
    //handle unexpected EOF
    if (!std::getline(configFile, _line))
        return false;
    _line = Utility::trimCommentsAndWhitespaces(_line);
    _lineNumber++;
    return true;
}

void Config::printServerConfig()
{
    LOG_DEBUG("Printing server config");
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