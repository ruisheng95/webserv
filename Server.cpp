#include "Server.hpp"
#include <iostream>

using std::string;
using std::vector;
using std::map;
using std::pair;
using std::cout;
using std::endl;

Server::Server() {}

Server::~Server() {}

string Server::get_host()
{
	return this->host;
}

string Server::get_port()
{
	return this->port;
}

string Server::get_server_name()
{
	return this->server_name;
}

map<string, string> &Server::get_error_pages()
{
	return this->error_pages;
}

string Server::get_client_max_body_size()
{
	return this->client_max_body_size;
}
vector<Location> &Server::get_location()
{
	return this->location;
}
	
void	Server::set_host(string host)
{
	this->host = host;
}

void Server::set_port(string port)
{
    this->port = port;
}

void Server::set_server_name(string server_name)
{
    this->server_name = server_name;
}

void Server::set_error_pages(map<string, string> error_pages)
{
    this->error_pages = error_pages;
}

void Server::set_client_max_body_size(string client_body_size)
{
    this->client_max_body_size = client_body_size;
}

void Server::set_get_location(vector<Location> location)
{
    this->location = location;
}

vector<pair<string, string> > Server::socket_addr; // need this for definition cannot just put inside header it wont compile rip

// void	print_server(Server &server)
// {
//     cout << "=======================================" << endl;
//     cout << "Server Details:" << endl;
//     cout << "Host: " << server.get_host() << endl;
//     cout << "Port: " << server.get_port() << endl;
//     cout << "Server Name: " << server.get_server_name() << endl;
//     cout << "Client Body Size: " << server.get_client_max_body_size() << endl;

// 	cout << endl;
//     cout << "Error Pages:" << endl;
// 	map<string,string> error_pages= server.get_error_pages();
//     for (map<string,string>::iterator it = error_pages.begin(); it != error_pages.end(); it++) {
//         cout << "Error Code: " << it->first << " -> Page: " << it->second << endl;
//     }

//     cout << endl;
// 	cout << "Locations: " << endl;
// 	for (vector<Location>::iterator it = server.get_location().begin(); it != server.get_location().end(); it++)
// 		it->print_location();

//     cout << "=======================================" << endl;
// }