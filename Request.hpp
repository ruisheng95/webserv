#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <map>

class Request
{
	private:
		std::string req_data;
		//parts of the request
		std::string method;
		std::string target;
		std::string http_ver;
		std::string host;
		std::string port;
		std::string content_length;
		std::map<std::string, std::string> header_fields;
		std::string body;

	public:
		Request();
		~Request();
		std::string	get_data();
		void	set_data(std::string newdata);
		void	set_http_ver(std::string http_ver);

		//get
		std::string	get_req_data();
		std::string	get_method();
		std::string	get_target();
		std::string	get_http_ver();
		std::string	get_host();
		std::string	get_port();
		std::string	get_content_length();
		std::map<std::string, std::string> &get_header_fields();
		std::string	get_body();


		//parsing
		void	parse_request_data_main(int socket_fd);
		void	parse_request_line(size_t &pos);
		void	parse_header_fields(size_t &pos);
		void	parse_body(size_t &pos, int socket_fd);
		void	parse_host_port(std::string str);
};

void	print_request(Request request);


#endif