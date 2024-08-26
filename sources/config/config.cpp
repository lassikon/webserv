/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: janraub <janraub@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 15:38:39 by janraub           #+#    #+#             */
/*   Updated: 2024/08/26 11:36:08 by janraub          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config.hpp"

int Config::_lineNumber = 0;
Config::Config(std::string configPath)
{
    _configFile.open(configPath);
    if (_configFile.fail())
        throw std::runtime_error("Parse: Could not open file at line " + std::to_string(_lineNumber));
    std::stringstream configFileBuffer;
    configFileBuffer << _configFile.rdbuf();
    parseConfigFile(configFileBuffer);
    validateServer(_servers);
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
            {
                LOG_WARN("Parse: Unclosed server block, ",_line, " at line ", _lineNumber);
                _blockStack.pop();
                configFile.clear();
                configFile.seekg(_pos);
            }
        }
        else
            LOG_WARN("Parse: Invalid block, ", _line, " at line ", _lineNumber);
        _pos = configFile.tellg();
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
            if (!_blockStack.empty() && (_blockStack.top() == "[route]"))
            {
                LOG_WARN("Parse: Unclosed route block, ", _line, " at line ", _lineNumber);
                _blockStack.pop();
                configFile.clear();
                configFile.seekg(_pos);
            }
        }
        else if (_line.compare("[/server]") == 0 && (!_blockStack.empty() && _blockStack.top() == "[server]"))
        {
            _blockStack.pop();
            addServerToMap(serverConfig);
            return;
        }
        else
            LOG_WARN("Parse: Invalid directive, ", _line, " in server block at line ", _lineNumber);
        _pos = configFile.tellg();
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
            LOG_WARN("Parse: Invalid directive, ", _line, " in route block at line ", _lineNumber);
        _pos = configFile.tellg();
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
            LOG_WARN("Parse: Invalid value, ", _line, " at line ", _lineNumber);
        else
            it_key->second(serverConfig, value);
    }
    else
        LOG_WARN("Parse: Invalid key, ", _line, " at line ", _lineNumber);
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
            LOG_WARN("Parse: Invalid value, ", _line, " at line ", _lineNumber);
        else
            it_key->second(routeConfig, value);
    }
    else
        LOG_WARN("Parse: Invalid key, ", _line, " at line ", _lineNumber);
}
// NGINX uses ip:port first, server_name second and default server last
void Config::addServerToMap(ServerConfig& serverConfig)
{
    //add server to map but with different ip, port or server name
    if (serverConfig.port == 0)
    {
        LOG_WARN("Parse: Missing port in server block at line ", _lineNumber);
        return;
    }
    std::string hostName;
    hostName = serverConfig.ipAddress + ":" + std::to_string(serverConfig.port);
    _servers.insert(std::pair<std::string, ServerConfig>(hostName, serverConfig));
}

// server struct setters
void Config::setIP(ServerConfig& server, std::string const & value)
{
    const std::regex ipPattern("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$");
    if (!std::regex_match(value, ipPattern))
    {
        LOG_WARN("Parse: Invalid IP, ", value, " at line ", _lineNumber);
        return;
    }
    if (!server.ipAddress.empty())
        LOG_WARN("Parse: IP already set, updating IP with line ", _lineNumber);
    else
        server.ipAddress = value;
}
void Config::setServerName(ServerConfig& server, std::string const & value)
{
    const std::regex serverNamePattern("^(\\w)[\\w-]{0,61}(\\w)(\\.[\\w-]{1,63})*$");
    if (!std::regex_match(value, serverNamePattern))
    {
        LOG_WARN("Parse: Invalid server name, ", value, " at line ", _lineNumber);
        return;
    }
    if (!server.serverName.empty())
        LOG_WARN("Parse: Server name already set, updating server name with line ", _lineNumber);
    else
        server.serverName = value;
}
void Config::setPort(ServerConfig& server, std::string const & value)
{
    const std::regex port_pattern("^[0-9]+$");
    if (!std::regex_match(value, port_pattern))
    {
        LOG_WARN("Parse: Invalid port, ", value, " at line ", _lineNumber);
        return;
    }
    else if (std::stoi(value) < 0 || std::stoi(value) > 65535)
    {
        LOG_WARN("Parse: Invalid port range, ", value, " at line ", _lineNumber);
        return;   
    }
    if (server.port != 0)
        LOG_WARN("Parse: Port already set, updating port with line ", _lineNumber);
    else
        server.port = std::stoi(value);
}
void Config::setErrorPages(ServerConfig& server, std::string const & value)
{
    std::stringstream ss(value);
    std::string error;
    std::getline(ss, error, ' ');
    const std::regex error_pattern("^[0-9]+$");
    if (!std::regex_match(error, error_pattern))
    {
        LOG_WARN("Parse: Invalid error code, ", error, " at line ", _lineNumber);
        return;
    }
    int error_code = std::stoi(error);
    std::getline(ss, error, ' ');
    std::filesystem::path exePath;
    exePath = getExePath(exePath);
    std::filesystem::path errorPath = exePath.append(error);
    if (!std::filesystem::exists(errorPath))
        LOG_WARN("Parse: Error page path not found, ", error, " at line ", _lineNumber);
    else
        server.pagesCustom[error_code] = error;
}
// client body size limit is set in bytes, kilobytes k,K, megabytes m,M, gigabytes g,G
void Config::setClientBodySizeLimit(ServerConfig& server, std::string const & value)
{
    const std::regex size_pattern("^[0-9]+[kKmMgG]?$");
    if (!std::regex_match(value, size_pattern))
    {
        LOG_WARN("Parse: Invalid client body size limit, ", value, " at line ", _lineNumber);
        return;
    }
    if (!server.clientBodySizeLimit.empty())
        LOG_WARN("Parse: Client body size limit already set, updating client body size limit with line ", _lineNumber);
    else
        server.clientBodySizeLimit = value;
}

// route struct setters
void Config::setLocation(RouteConfig& route, std::string const & value)
{
    if (!route.location.empty())
        LOG_WARN("Parse: Location already set, updating location with line ", _lineNumber);
    route.location = value;
}

void Config::setMethods(RouteConfig& route, std::string const & value)
{
    std::stringstream ss(value);
    std::string method;
    while (std::getline(ss, method, ','))
    {
        method = Utility::trimCommentsAndWhitespaces(method);
        if (method.empty())
            LOG_WARN("Parse: Invalid method, ", method, " at line ", _lineNumber);
        else if (method != "GET" && method != "POST" && method != "DELETE")
        {
            LOG_WARN("Parse: Invalid method, ", method, " at line ", _lineNumber);
            continue;
        }
        else 
            route.methods.push_back(method);
    }
}

void Config::setRoot(RouteConfig& route, std::string const & value)
{
    std::filesystem::path exePath;
    exePath = getExePath(exePath);
    std::filesystem::path rootPath = exePath.append(value);
    if (!std::filesystem::exists(rootPath))
    {
        LOG_WARN("Parse: Root path not found, ", value, " at line ", _lineNumber);
        return;
    }
    if (!route.root.empty())
        LOG_WARN("Parse: Root already set, updating root with line ", _lineNumber);
    else
        route.root = value;
}

void Config::setDirectoryListing(RouteConfig& route, std::string const & value)
{
    if (value == "on")
        route.directoryListing = true;
    else if (value == "off")
        route.directoryListing = false;
    else
        LOG_WARN("Parse: Invalid directory listing, ", value, " at line ", _lineNumber);
}

void Config::setDefaultFile(RouteConfig& route, std::string const & value)
{
    if (!route.defaultFile.empty())
        LOG_WARN("Parse: Default file already set, updating default file with line ", _lineNumber);
    route.defaultFile = value;
}

void Config::setUploadPath(RouteConfig& route, std::string const & value)
{
    std::filesystem::path exePath;
    exePath = getExePath(exePath);
    std::filesystem::path uploadPath = exePath.append(value);
    if (!std::filesystem::exists(uploadPath))
    {
        LOG_WARN("Parse: Upload path not found, ", value, " at line ", _lineNumber);
        return;
    }
    if (!route.uploadPath.empty())
        LOG_WARN("Parse: Upload path already set, updating upload path with line ", _lineNumber);
    else
        route.uploadPath = value;
}
//either a path or a redirect url
void Config::setRedirect(RouteConfig& route, std::string const & value)
{
    if (value.front() == '/')
    {
        std::filesystem::path exePath;
        exePath = getExePath(exePath);
        std::filesystem::path redirectPath = exePath.append(value);
        if (!std::filesystem::exists(redirectPath))
        {
            LOG_WARN("Parse: Redirect path not found, ", value, " at line ", _lineNumber);
            return;
        }
    }
    if (!route.redirect.empty())
        LOG_WARN("Parse: Redirect already set, updating redirect with line ", _lineNumber);
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

std::filesystem::path & Config::getExePath(std::filesystem::path & path)
{
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path exePath = currentPath / "webserv";
    if (!std::filesystem::exists(exePath))
        LOG_ERROR("Parse: Could not find executable at ", exePath);
    path = std::filesystem::canonical(exePath).parent_path().string();
    return (path);
}

int Config::getLineNumber() const
{
    return _lineNumber;
}

std::map<std::string, ServerConfig> & Config::getServers()
{
    return _servers;
}

void Config::validateServer(std::map<std::string, ServerConfig> & servers)
{
    for (auto serverIt = servers.begin(); serverIt != servers.end(); serverIt++)
    {
        auto& server = serverIt->second;
        for (auto routeIt = server.routes.begin(); routeIt != server.routes.end();)
        {
            if (routeIt->location.empty())
            {
                LOG_WARN("Parse: Missing location in route block, deleting route...");
                routeIt = server.routes.erase(routeIt);
            }
            else
                ++routeIt;
        }
    }
}

void Config::printServerConfig()
{
    LOG_DEBUG("Printing server config");
    int serverCount = 1;
    for (const auto& server_pair : _servers)
    {
        const auto& server = server_pair.second;
        LOG_INFO("#Server: ", serverCount);
        LOG_INFO("Server host: ", server.ipAddress);
        LOG_INFO("Server port: ", server.port);
        LOG_INFO("Server name: ", server.serverName);
        LOG_INFO("Server error pages: ");
        for (const auto& error : server.pagesCustom)
        {
            LOG_INFO(error.first,"  path: ",error.second);
        }
        LOG_INFO("Server client body size limit: ", server.clientBodySizeLimit );
        int routeCount = 1;
        for (const auto& route : server.routes)
        {
            LOG_INFO("#Server: ", serverCount, " route: ", routeCount);
            LOG_INFO("  Route location: ",route.location);
            LOG_INFO("  Route methods: ");
            for (const auto& method : route.methods)
            {
                LOG_INFO("    ", method);
            }
            LOG_INFO("  Route cgi: ");
            for (const auto& cgi : route.cgi)
            {
                LOG_INFO("    ", cgi);
            }
            LOG_INFO("  Route root: ", route.root);
            LOG_INFO("  Route directory listing: ", route.directoryListing);
            LOG_INFO("  Route default file: ", route.defaultFile);
            LOG_INFO("  Route upload path: ",route.uploadPath);
            LOG_INFO("  Route redirect: ",route.redirect);
            routeCount++;
        }
        serverCount++;
    }
}