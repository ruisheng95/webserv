#include "Config.hpp"
#include "HTTP.hpp"
#include <stdexcept>
#include <iostream>
#include <signal.h>

void	signal_handler(int sig)
{
	(void) sig;
	HTTP::is_run = false;
}

//damn i wish every file and code can be as short as this
int main(int argc, char **argv)
{
	Config config("testconfig");
	HTTP	http;
	try
	{
		if(argc < 2)
			throw std::invalid_argument("Error: wrong number of args");
		signal(SIGINT, signal_handler);
		http._run_webserv(argv);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}