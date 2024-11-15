# C++ Webserver

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Configuration](#configuration)
- [Performance](#performance)
- [Contact](#contact)

## Overview

This **C++ Webserver** is a robust, efficient server implementation that supports **static file hosting** and **CGI scripts**. Built with **object-oriented design** principles in C++, the server is tailored for high performance using **epoll** for efficient I/O polling. It also includes features like **session management with cookies** and a configurable **logging system** with multiple log levels (`TRACE`, `DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL`).

this webserver can host multiple servers simultaneously, each with its own set of routes and support for **virtual servers**, .

## Features

- **Static File Hosting**: Serve static files efficiently.
- **CGI Script Handling**: Run CGI scripts with seamless integration.
- **HTTP/1.1 Support**: Implements the GET, POST, and DELETE methods.
- **Session Management**: Simple session management using cookies.
- **Virtual Servers**: Host multiple servers, each with custom routes and configurations.
- **Efficient Polling**: Uses `epoll` for high-performance I/O.
- **Configurable Logging**: Logs requests and errors with multiple severity levels.

## Requirements

- **Operating System**: Linux
- **Compiler**: A modern C++ compiler with support for C++17 or later
- **Libraries**: No external libraries are required; everything is implemented from scratch.

## Installation

1. **Clone the repository**:

   ```bash
   git clone git@github.com:lassikon/webserv.git
   cd webserver
   ```

2. **Compile the project**:

**Runs easy.conf configuration file by default**

```bash
make run
```

3. **Stop the server**:  
   Use `Ctrl+C` to terminate the process.

## Usage

Once the server is running, it will serve requests based on the configurations specified in the config file. For example:

- **Static File Hosting**: Access hosted files via `http://your-server-ip:port/static-file-path`.
- **CGI Script Handling**: Execute CGI scripts via routes configured in the `config` file.

### Logging Levels

Logs are saved in the server log file, with levels ranging from `TRACE` for detailed debugging information to `FATAL` for critical errors.

## Configuration

The server uses a configuration file that supports multiple servers and routes. Below is an example format:

```ini
[server]
server_ip: 127.0.0.1
server_port: 3490
server_name: default-server.com
error_page: 404 pagesCustom/404.html
cgi_interpreter: py /usr/bin/python3
client_body_size_limit: 100M

[route]
location: / 
methods: GET, POST, DELETE
root: ./webroot/website0

[route]
location: /cgi-bin/
methods: GET, POST
root: ./webroot/website0

[server]
server_ip: 127.0.0.1
server_port: 3490
```

### Key Directives

- **Server Level**:
  - `host`: The IP address to bind the server.
  - `port`: The port number to listen on.
  - `server_name`: The server name used.
  - `error_page`: Set up a custom error page.
  - `cgi_interpreter`: Interpreter for cgi , python, php...
  - `client_body_size_limit`: Client bodysize limit

- **Route Level**:
  - `location`: The URL path for the route.
  - `root`: The root directory for static files.
  - `methods`: The methods allowed
  - `directory_listing`: Sets up directory listing when no index file or default file available.
  - `default_file`: Sets up default file to serve if requests a directory location.
  - `upload_path`: Upload directory for uploaded files.
  - `redirect`: Redirects to URL path 


## Performance

This webserver was stress-tested using **Siege**, demonstrating **100% availability** even under high loads, including scenarios with complex CGI script handling. The use of `epoll` ensures efficient resource usage and scalability under concurrent requests.

## Contact

**Your Name**  
[Janrau's Email](janraup356@gmail.com)  
[Janrau's LinkedIn](https://linkedin.com/in/janrau-beray)
