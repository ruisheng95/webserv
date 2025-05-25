#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string>
#include <map>
#include <vector>
#include "Request.hpp"
#include "Response.hpp"

class Location;
class Server;
class Request;
class Response;

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