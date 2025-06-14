#include "Socket.hpp"
#include "Server.hpp"
#include <cstring>
#include <netdb.h>
#include <stdexcept>
#include <string>
#include <sys/poll.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <limits.h>
#include <fcntl.h>

using std::vector;
using std::string;
using std::pair;
using std::cout;
using std::endl;

#define LISTEN_BACKLOG 1000

ssize_t my_atoi(std::string s);

Socket::Socket() {}

Socket::~Socket() {}

// Define static member variables
vector<Socket> Socket::io_connections;
vector<struct pollfd> Socket::poll_socket_fds;
vector<int> Socket::listen_socket_fds;

Request &Socket::get_req()
{
	return this->request;
}

void	Socket::set_sock_fd(int sockfd)
{
	this->sock_fd = sockfd;
}


////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////socket setup////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void Socket::main_setup_socket(vector<pair<int, struct addrinfo> > &all_sockets_list)
{
	for(vector<pair<string, string> >::iterator it = Server::socket_addr.begin(); it != Server::socket_addr.end(); it++)
	{
		int sockfd;
		struct addrinfo *reso = NULL;
		sockfd = setup_socket(it->first, it->second, &reso); //binding and other socket programs
		add_new_socket_to_poll(sockfd, POLLIN); //add new socket to poll monitor pool
		all_sockets_list.push_back(std::make_pair(sockfd, *reso)); //add new socket as well as all its information to the list
		freeaddrinfo(reso);
	}
}

int	Socket::setup_socket(string host, string port, struct addrinfo **reso)
{
	int				sockfd;
	struct addrinfo	hints; //for getaddrinfo later

	//init hint struct
	memset(&hints, 0, sizeof(hints));
	//fill in these so getaddrinfo can return the resolved addresses for these requirements(the rest will be auto filled in by getinfoaddr)
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0; //0 means we let getaddrinfo auto decide
	hints.ai_flags = AI_NUMERICSERV;

	//getaddrinfo
	if(getaddrinfo(host.c_str(), port.c_str(), &hints, reso) < 0)
		throw std::runtime_error("Error: getaddrinfo failed");

	//create socket
	if((sockfd = socket((*reso)->ai_family, (*reso)->ai_socktype, (*reso)->ai_protocol)) < 0)
		throw std::runtime_error("Error: Socket creation failed");

	//set socket nonblock
	int flags;
    if ((flags = fcntl(sockfd, F_GETFL, 0)) == -1)
	{
        close(sockfd);
        throw std::runtime_error("Error: fcntl(F_GETFL) failed");
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
        close(sockfd);
        throw std::runtime_error("Error: fcntl(O_NONBLOCK) failed");
    }

	//set socket options (so we can bind without time delay after socket is closed)
	int opt = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		close(sockfd);
		throw std::runtime_error("Error: Setsockopt failed");
	}	
	
	//bind socket
	if(bind(sockfd, (*reso)->ai_addr, (*reso)->ai_addrlen) < 0)
	{
		close(sockfd);
		throw std::runtime_error("Error: Bind socket failed");
	}

	//mark socket as listening socket
	if(listen(sockfd, LISTEN_BACKLOG) < 0) //listen backlog is the total maximum total amm of incoming connections we can have
	{
		close(sockfd);
		throw std::runtime_error("Error: Listen socket failed");
	}
	
	//final stuffs
	cout << "Listening on port: " << port << "...." << endl;
	listen_socket_fds.push_back(sockfd);
	return(sockfd);
}

//notes:
//AF_INET is to specift ipv4 socket
//SOCK_STREAM is to specify TCP
//AI_NUMERICSERV tells getaddrinfo to treat the port as numbers instead of service names (such as http, where equates to port 80)

//getaddrinfo inserts a resolved address linked list (reso) which contains the information about addresses that fit our requirements (the hints we inserted)
//lets say we type host is localhost, then it returns a linked list fill wif information abt addresses wif 127.0.0 (for IPv4) 
//after that we can use the information in the struct it returns to help us create and bind our sockets.



///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////process request////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

int		Socket::find_content_length(string request)
{
	size_t pos = request.find("Content-Length:");
	if(pos == std::string::npos)
		return 0;
	size_t end_pos = request.find_first_of("\r\n", pos);
	string contentlen = request.substr(pos, end_pos - pos);
	return my_atoi(contentlen.substr(16, contentlen.size() - 16));
}

void	Socket::receive_data(Socket &socket)
{
	char buffer[900000] = {0}; //HARDCODE AH
	int contentlen = 1;
	int bodystart = -1;
	while(contentlen > 0)
	{
		ssize_t bytes_received = recv(socket.sock_fd, buffer, sizeof(buffer), 0);
		// Uncomment to emulate recv failure
		// int num = rand() % 2;
		// if (num == 1) {
		// 	bytes_received = -1;
		// }
		if(bytes_received < 0) {
			socket.get_req().set_data("");
			return ;
		}
		socket.get_req().set_data(socket.get_req().get_data().append(buffer, bytes_received));
		string curr_req = socket.get_req().get_data();
		contentlen = find_content_length(curr_req);
		if (contentlen == 0)
			break;
		bodystart = curr_req.find("\r\n\r\n") + 4;
		contentlen -= (curr_req.size() - bodystart);
	}
	socket.get_req().parse_request_data_main(socket.sock_fd);
}

void	Socket::process_req_POLLIN_listen_socket(int i ,vector<pair<int, struct addrinfo> > &sockets_addrinfo)//process listening socket
{
	struct addrinfo *res = NULL;
	Socket connection_socket;
	int accept_socket_fd;
	for(size_t i = 0; i < sockets_addrinfo.size(); i++)
	{
		if(sockets_addrinfo[i].first == poll_socket_fds[i].fd)
		{
			res = &(sockets_addrinfo[i].second); 
			break;
		}
	}
	if(res == NULL)
		throw std::runtime_error("Error: can't find socket addinfo");
	if((accept_socket_fd = accept(poll_socket_fds[i].fd,  (struct sockaddr*) &res->ai_addr, &res->ai_addrlen)) <= 0)
		throw std::runtime_error("Error: accept socket failed");
	
	//set nonblock
	// int flags;
    // if ((flags = fcntl(accept_socket_fd, F_GETFL, 0)) == -1)
	// {
    //     close(accept_socket_fd);
    //     throw std::runtime_error("Error: fcntl(F_GETFL) failed");
    // }
    // if (fcntl(accept_socket_fd, F_SETFL, flags | O_NONBLOCK) < 0)
	// {
    //     close(accept_socket_fd);
    //     throw std::runtime_error("Error: fcntl(O_NONBLOCK) failed");
    // }

	//add accept socket to our new connection
	connection_socket.set_sock_fd(accept_socket_fd);
	io_connections.push_back(connection_socket);

	//add accept socket to poll
	add_new_socket_to_poll(accept_socket_fd, POLLIN);
}

void	Socket::process_req_POLLIN_connection_socket(int i)//process connection socket
{
	int connect_index = get_io_connection(poll_socket_fds[i].fd);
	receive_data(io_connections[connect_index]);
	update_fd_event(poll_socket_fds[i].fd, POLLIN | POLLOUT); // need to update here instead of when accept() returns the socket if not will have error
	//std::cout << "exit pollin" << std::endl;
}

void	Socket::process_req_POLLOUT(int i, vector<Server> Servers) //output stuff and close socket
{
	//std::cout << "enter pollout" << std::endl;
	int connect_index = get_io_connection(poll_socket_fds[i].fd);
	Socket socket = io_connections[connect_index];
	if(socket.get_req().get_req_data().length() == 0) {
		this->response.error_response_function();
	} else {
		this->response.main_response_function(socket.get_req(), Servers);
	}
	ssize_t result = send(socket.sock_fd, this->response.get_response_data().c_str(), this->response.get_response_data().size(), 0);
	if (result == -1) {
		cout << "Send failed for client" << endl;
	}
	// Regardless Send success or fail, the fd will be closed
	close_fd(poll_socket_fds[i].fd, i); //see explanation in function
}

//important note, never use "this" here
void	Socket::process_req(vector<pair<int, struct addrinfo> > &sockets_addrinfo, vector<Server>Servers)
{
	//find the port that has a req
	for(size_t i = 0; i < poll_socket_fds.size(); i++)
	{
		if(poll_socket_fds[i].revents == POLLIN)
		{
			if(std::find(listen_socket_fds.begin(), listen_socket_fds.end(), poll_socket_fds[i].fd) != listen_socket_fds.end())
				process_req_POLLIN_listen_socket(i, sockets_addrinfo);
			else
				process_req_POLLIN_connection_socket(i);
		}
		else if (poll_socket_fds[i].revents == POLLOUT)
			process_req_POLLOUT(i, Servers);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////socket utils////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void	Socket::add_new_socket_to_poll(int fd, int ev)
{
	struct pollfd newfd;
	memset(&newfd, 0, sizeof(newfd));
	newfd.fd = fd;
	newfd.events = ev;
	poll_socket_fds.push_back(newfd);
}

void Socket::close_fd(int fd, int fd_index)
{
	io_connections.erase(get_io_connection(fd) + io_connections.begin());
	poll_socket_fds.erase(poll_socket_fds.begin() + fd_index);
	close(fd);

//need to specially close not just the fd but everything abt it especially the ioconnection part;
//cuz the io connection part has information abt the request previously
//so if we only close the fd without closing this, when we open again the previous request will be used,
//hence causing the next fd u open will have the same request
//lets say we accepted fd 5, send the response, and close ONLY THE FD
//the ioconnection isnt removed, so the next time we accept again wif fd 5, we will end up using the old socket object in ioconnections
}

int	Socket::get_io_connection(int fd)
{
	for(size_t i = 0; i < io_connections.size(); i++)
	{
		if(io_connections[i].sock_fd == fd)
			return i;
	}
	throw std::runtime_error("Error: cannot find io connection");
	return -1;
}

void	Socket::update_fd_event(int fd, int ev)
{
	for(size_t i = 0; i < poll_socket_fds.size(); i++)
	{
		if(poll_socket_fds[i].fd == fd)
			poll_socket_fds[i].events = ev;
	}
}