#ifndef CGI_HPP
#define CGI_HPP

#include <string>

class Request;
class Response;
class Location;
class Server;

class Cgi
{
	private: 

	public:
	Cgi();
	~Cgi();
	void		Cgi_main(Request &request, Response &response, Location &location, Server &server);
	char 	**config_env(Request &request);
	std::string	get_cgi_output(int pipefd);
	int 	check_cgi_hang(Request &request, Response &response, Location &location, Server &server);
	int		check_valid_response(std::string str);
};

#endif