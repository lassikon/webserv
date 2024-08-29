#pragma once

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

#include <Logger.hpp>
#include <Utility.hpp>
#include <Config.hpp>

class Client;

class Response {
	private:
		int statusCode;
		std::string statusMessage;
		std::string responseLine;
		std::map<std::string, std::string> headers;
		std::vector<char> body;
		bool transferEncodingChunked;
		std::shared_ptr<Config> config;
	public:
		Response();
		~Response();
		void makeResLine(std::ostringstream& oBuf);
		void makeHeaders(std::ostringstream& oBuf);
		void makeBody(std::ostringstream& oBuf);

		void setConfig(std::shared_ptr<Config> config) { this->config = config; }
};