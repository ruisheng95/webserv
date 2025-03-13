#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <map>
#include "Location.hpp"

class Server
{
	private:
		std::string host;
		std::string port;
		std::string server_name;
		std::map<std::string, std::string> error_pages;
		std::string client_max_body_size;
		std::vector<Location> location;

	public:
		Server();
		~Server();
		std::string get_host();
		std::string get_port();
		std::string get_server_name();
		std::map<std::string, std::string> &get_error_pages(); //need to put reference if not i cant assign values to it
		std::string get_client_max_body_size();
		std::vector<Location> &get_location();

		void	set_host(std::string host);
		void	set_port(std::string port);
		void	set_server_name(std::string server_name);
		void	set_error_pages(std::map<std::string, std::string> error_pages);
		void	set_client_max_body_size(std::string client_body_size);
		void	set_get_location(std::vector<Location> location);

		static std::vector<std::pair<std::string, std::string> > socket_addr; //to keep track of all the sockets we need to monitor (TODO set private?)
};

void	print_server(Server &server);

#endif