#include "HTTP.hpp"
#include "Socket.hpp"
#include "Config.hpp"
#include <stdexcept>
#include <sys/poll.h>
#include <unistd.h>

using std::string;
using std::vector;
using std::pair;

bool HTTP::is_run = true;

HTTP::HTTP() {}

HTTP::~HTTP() {}

vector<Server> &HTTP::get_servers()
{
	return(this->Servers);
}

vector<pair<int, struct addrinfo> > &HTTP::get_all_sockets_list()
{
	return(this->all_sockets_list);
}

void	HTTP::poll_loop()
{
	int poll_status;
	Socket sock;
	while(is_run)
	{
		poll_status = poll(Socket::poll_socket_fds.data(), Socket::poll_socket_fds.size(), -1);//poll will always be 0 if nth is detected, -1 to tell poll to wait infinitely wif no max time
		if(poll_status == 0)
			continue;
		else if (poll_status < 0)
		{
			if (is_run)
				throw std::runtime_error("Error: poll error");
			else {
				for(size_t i = 0; i < Socket::listen_socket_fds.size(); i++) {
					close(Socket::listen_socket_fds[i]);
				}
			}
		}
		else
			sock.process_req(this->all_sockets_list, Servers);
	}
}

void	HTTP::_run_webserv(char **argv)
{
	Config config(static_cast<string>(argv[1]));
	Socket sock;

	//parsing
	config.main_parse_function(Servers);
	
	//setting up sockets
	sock.main_setup_socket(this->all_sockets_list);
	
	//main loop that waits for connections and process it
	poll_loop();
}