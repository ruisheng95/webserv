#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string>
#include <map>
#include <vector>

class Location;
class Server;

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

class Socket
{
	private:
		int	sock_fd;
		Request request;
		Response response;

	public:
		Socket();
		~Socket();

		static std::vector<struct pollfd> poll_socket_fds;
		static std::vector<int> listen_socket_fds;
		static std::vector<Socket> io_connections;

		//get
		Request	&get_req();

		//setup socket
		void	main_setup_socket(std::vector<std::pair<int, struct addrinfo> > &all_sockets_list);
		int	setup_socket(std::string host, std::string port, struct addrinfo **reso);

		//request stuff
		void	receive_data(Socket &socket);
		void 	better_receive_data(Socket &socket);
		void	process_req_POLLIN_connection_socket(int i);
		void	process_req_POLLIN_listen_socket(int i, std::vector<std::pair<int, struct addrinfo> > &sockets_addrinfo);
		void	process_req_POLLOUT(int i, std::vector<Server> Servers); //output stuff and close socket
		void	process_req(std::vector<std::pair<int, struct addrinfo> > &sockets_addrinfo, std::vector<Server>Servers);

		//utils
		void	add_new_socket_to_poll(int fd, int ev);
		void	update_fd_event(int fd, int ev);
		void	set_sock_fd(int sockfd);
		int		get_io_connection(int fd);
		void	close_fd(int fd, int fd_index);
		int		find_content_length(std::string request);
};

#endif