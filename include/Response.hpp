#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include "Server.hpp"
#include "Location.hpp"

class Location;
class Server;
class Request;

class Response //include this here first cuz somehow incomplete type
{
	private:
	std::string response_data;
	std::string errorCode;

	public:
		Response();
		~Response();
		void	set_response(std::string newResponse);
		std::string	get_response_data();

		void	main_response_function(Request request, std::vector<Server> &Servers);
		void	error_response_function();

		void	do_indexing(Request request, Server &server, Location *location, std::string resource_path);
		void	get_file_contents(Request request, Server &server, std::string resource_path);

		void	handle_get(Request request, Server &server);
		void	handle_post(Request request, Server &server);
		void	handle_delete(Request request, Server &server);
		void	handle_error(Request request, std::string error_code, Server &server);
		void	handle_return(Request request, Server &server, Location &location);
		void	handle_autoindex(Request request, Server &server, Location &location, std::string req_path);

		std::string	get_error_page(std::string error_code, Server &server);
		std::string	get_start_line(Request request, std::string code, Server &server);
		std::string	get_file_type(std::string path);
		std::string	parse_resources(std::string path);
		std::string	get_full_resource_path(Request request, Location &location);
		std::string	urlDecode(std::string &str);
		std::string	get_code_string(std::string error_code);
		Location *get_location(Request request, Server &server);
		int		check_allowed_methods(std::string method, Location &location);
		std::string	get_headers(std::string content, std::string content_type);
		Server	&find_server(Request request, std::vector<Server> &Servers);
		int		check_request_body_valid(Request &request);
};
// class Response //include this here first cuz somehow incomplete type
// {
// 	private:
// 		string response_data;

// 	public:
// 		Response();
// 		~Response();
// 		void	set_response(string newResponse);
// 		string	get_response_data();

// 		void	main_response_function(Request request, vector<Server> &Servers);

// 		string	get_start_line(Request request, string code, Server &server);
// 		string	get_code_string(string error_code);
// 		string	get_error_page(string error_code, Server &server);
// 		string	get_headers(string content);
// 		void	handle_get(Request request, Server &server);
// 		void	handle_post(Request request, Server &server);
// 		void	handle_delete(Request request, Server &server);
// 		void	handle_error(Request request, string error_code, Server &server);
// 		Server	&find_server(Request request, vector<Server> &Servers);
// };


#endif