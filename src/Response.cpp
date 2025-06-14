#include "Autoindex.hpp"
#include "Socket.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "Cgi.hpp"
#include <dirent.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <iostream>
using std::vector;
using std::string;
using std::map;

std::string my_itoa(size_t i);

ssize_t my_atoi(std::string s);

int my_hex_to_int(std::string s);

Response::Response() {}

Response::~Response() {}

void	Response::set_response(string newResponse)
{
	this->response_data = newResponse;
}

string	Response::get_response_data()
{
	return this->response_data;
}
////////////////////
////////main////////
////////////////////

void	Response::main_response_function(Request request, vector<Server> &Servers)
{
	Server &respective_server = find_server(request, Servers);
	this->errorCode = "";
	this->response_data = "";
	if(request.get_method() == "GET")
		handle_get(request, respective_server);
	else if(request.get_method() == "POST")
		handle_post(request, respective_server);
	else if(request.get_method() == "DELETE")
		handle_delete(request, respective_server);
	else
		handle_error(request, "405", respective_server);
	//std::cout << this->response_data << std::endl;
}

void	Response::error_response_function()
{
	Request dummyRequest;
	Server dummyServer;
	dummyRequest.set_http_ver("HTTP/1.1");
	handle_error(dummyRequest, "500", dummyServer);
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////response making helpers/////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

string	Response::get_start_line(Request request, string code, Server &server)
{
	(void)server;
	string start_line = request.get_http_ver() + " " + code + " " + get_code_string(code) + "\r\n";
	return start_line;
}

string	Response::get_headers(string content, string content_type)
{
	string res = "Content-Type: " + content_type + "\r\nContent-Length: " + my_itoa(content.size()) + "\r\nConnection: keep-alive\r\n\r\n";
	return res;
}


////////////////////////////////////////////////
//////////////////error stuffs//////////////////
////////////////////////////////////////////////

string	Response::get_error_page(string error_code, Server &server)
{
	this->errorCode = error_code;
	string error_page_contents;
	string buffer;
	for(map<string, string>::iterator it = server.get_error_pages().begin(); it != server.get_error_pages().end(); it++)
	{
		if(it->first == error_code)
		{
			string filepath = "." + it->second;
			std::ifstream infile(filepath.c_str());
			if(!infile.is_open()) {
				return "";
			}
			// Clear error code, so that response body is from error_page_contents
			this->errorCode = "";
			while(getline(infile, buffer))
				error_page_contents.append(buffer);
		}
	}
	return error_page_contents;
}

void	Response::handle_error(Request request, string error_code, Server &server)
{
	// Reset errorCode in Response, to be used in get_error_page
	this->errorCode = "";
	string start_line = get_start_line(request, error_code, server);
	string error_page_contents = get_error_page(error_code, server);
	if(this->errorCode != "")
	{
		error_page_contents += "<!DOCTYPE html>";
		error_page_contents += "<head>";
		error_page_contents += "<title>" + get_code_string(error_code) + "</title>";
		error_page_contents += "</head>";
		error_page_contents += "<body>";
		error_page_contents += "<center>";
		error_page_contents += "<h1> Error: " + error_code + " " + get_code_string(error_code) + "</h1>";
		error_page_contents += "</center>";
		error_page_contents += "</body>";
		error_page_contents += "</html>";
	}
	string headers = get_headers(error_page_contents, "text/html");
	this->response_data = start_line + headers + error_page_contents;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////indexing////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


void	Response::do_indexing(Request request, Server &server, Location *location, string resource_path)
{
	string file_contents;
	vector<string>::iterator it = location->get_index().begin();
	vector<string>::iterator ite = location->get_index().end();
	while(it != ite)
	{
		//Empty file is a valid content. Check error code instead
		//std::cout << resource_path + *it << std::endl;
		file_contents = parse_resources(resource_path + *it);
		if (this->errorCode != "" && it + 1 != ite)
		{
			//If error, and not last index file (so that error code can go to handle_error func)
			//Reset error code, try next file
			this->errorCode = "";
			it++;
		}
		else
			break;
	}
	if (this->errorCode != "")
		handle_error(request, this->errorCode, server);
	else if(location->get_autoindex() == true)
		handle_autoindex(request, server, *location, resource_path);
	else if(file_contents != "")
		this->response_data = get_start_line(request, "200", server) + get_headers(file_contents, get_file_type(resource_path + *it)) + file_contents;
	else
		handle_error(request, "403", server);
}

void	Response::handle_autoindex(Request request, Server &server, Location &location, string path)
{
	if(location.get_autoindex() == true)
	{
		DIR *dir = opendir(path.c_str());
		if(!dir)
			handle_error(request, "404", server);
		else
		{
			string response_body;
			string dir_name = path.substr(1, path.length() - 1); // to remove the slash
			Autoindex auto_index;

			auto_index.generate_autoindex_page_first_part(response_body, dir_name); //generate the top part of the html page (styling, headers....)
			auto_index.generate_autoindex_page_second_part(response_body, dir, path); //generate the body part (readdir and listing the filees)
			closedir(dir);
			this->response_data = get_start_line(request, "200", server) + get_headers(response_body, "text/html") + response_body;
		}
	}
	else
		handle_error(request, "404", server);
}
///////////////////////////////////////////////////////////////////////////
///////////////////////////getting file contents///////////////////////////
///////////////////////////////////////////////////////////////////////////

string	Response::parse_resources(string path)
{
	string res;
	string buffer;
	if (access(path.c_str(), F_OK) != 0) {
		this->errorCode = "404";
		return "";
	}
	else if (access(path.c_str(), R_OK) != 0) {
		this->errorCode = "403";
		return "";
	}
	std::ifstream infile(path.c_str());
	if(!infile.is_open()) {
		this->errorCode = "404";
		return "";
	}
	while(getline(infile, buffer))
		res += buffer + "\n";
	return res;
}

string	Response::get_file_type(string path)
{
	size_t dotpos = path.find_first_of(".", 1);
	if(dotpos == string::npos)
		return("plain/text");
	string type = path.substr(dotpos + 1, path.length() - dotpos - 1);

	if(type == "html")
		return "text/html"; //cannot reverse the order if not will have some weird behaviour
	else if (type == "css")
		return "text/css";
	else if (type == "js")
        return "text/javascript";
	else if (type == "jpeg" || type == "jpg")
		return "image/jpeg";
	else if (type == "png")
		return "image/png";
	else if (type == "gif")
		return "image/gif";
	else if (type == "txt")
		return "text/plain";
	else
		return "text/plain";
}

void	Response::get_file_contents(Request request, Server &server, string resource_path)
{
	string file_contents = parse_resources(resource_path);
	if(this->errorCode != "")
		handle_error(request, this->errorCode, server);
	else
		this->response_data = get_start_line(request, "200", server) + get_headers(file_contents, get_file_type(resource_path)) + file_contents;
}

/////////////////////////////////////////////////////////////////////
/////////////////////////handle methods fts//////////////////////////
/////////////////////////////////////////////////////////////////////

void	Response::handle_get(Request request, Server &server)
{
	Location *location = get_location(request, server);
	Cgi cgi;

	if(!location)
		handle_error(request, "403", server);
	else if(check_allowed_methods("GET", *location) == 0)
		handle_error(request, "405", server);
	else if(location->get_return() != "")
		handle_return(request, server, *location);
	else
	{
		string resource_path = get_full_resource_path(request, *location);
		if(resource_path[resource_path.length() - 1] == '/')// checks to see if our request doesnt specify a file, supposed to send in an index file
			do_indexing(request, server, location, resource_path);
		else if(location->get_cgi_pass() != "")
			cgi.Cgi_main(request, *this, *location, server);
		else
			get_file_contents(request, server, resource_path);
	}
}

void	Response::handle_post(Request request, Server &server)
{
	Location *location = get_location(request, server);
	//std::cout << "contentlen: " << request.get_content_length() << std::endl;
	//std::cout << "cbs: " << my_atoi(server.get_client_max_body_size()) << std::endl;
	if(!location)
		handle_error(request, "403", server);
	else if(check_allowed_methods("POST", *location) == 0) //check allowed methods
		handle_error(request, "405", server);
	else if(location->get_return() != "")
		handle_return(request, server, *location); //check return
	else if(request.get_content_length() != "" && server.get_client_max_body_size() != ""
			&& my_atoi(request.get_content_length()) > my_atoi(server.get_client_max_body_size()))
		handle_error(request, "413", server);
	else if(location->get_cgi_pass() == "")
		handle_error(request, "404", server);
	else if(check_request_body_valid(request) == 0)
		handle_error(request, "422", server);
	else
	{
		Cgi cgi;
		cgi.Cgi_main(request, *this, *location, server);
	}
}

void	Response::handle_delete(Request request, Server &server)
{
	Location *location = get_location(request, server);
	if(!location)
		handle_error(request, "403", server);
	else if(check_allowed_methods("DELETE", *location) == 0)
		handle_error(request, "405", server);
	else if(location->get_return() != "")
		handle_return(request, server, *location);
	else
	{
		Cgi cgi;
		cgi.Cgi_main(request, *this, *location, server);
	}
}

/////////////////////////////////////////////
/////////////////other utils/////////////////
/////////////////////////////////////////////

string	Response::get_code_string(string code)
{
	if(code == "200")
		return ("OK");
	if(code == "301")
		return("Moved Permenantly");
	if(code == "302")
		return ("Found");
	if(code == "307")
		return ("Temporary Redirect");
	if(code == "403")
		return("Forbidden");
	if(code == "404")
		return("Not found");
	if(code == "405")
		return("Method not allowed");
	if(code == "413")
		return("Payload too large");
	if(code == "422")
		return("Unprocessable Entity");
	if(code == "500")
		return("Internal Server Error");
	return("Unknown Error");
}

string Response::urlDecode(string &str)
{
	string res;

	for(size_t i = 0; i < str.size(); i++)
	{
		if(str[i] == '%' && i < str.size() - 2)
		{
			string temp_hex_str = str.substr(i + 1, 2);
			// Not available on c++98
			// int hex_value = stoi(temp_hex_str, nullptr, 16); //stoi i love u sm idh to manually putnbr base
			int hex_value = my_hex_to_int(temp_hex_str);
			res += static_cast<char>(hex_value);
			i += 2;
		}
		else if(str[i] == '+')
			res += ' ';
		else
			res += str[i];
	}
	return res;
}

Server	&Response::find_server(Request request, vector<Server>& Servers)
{
	// Return the last successful match server (assume Servers vector not empty)
	Server &server = Servers[0];
	for(size_t i = 0; i < Servers.size(); i++)
	{
		if(request.get_port() == Servers[i].get_port())
		{
			if (Servers[i].get_server_name() == ""
			|| (Servers[i].get_server_name() != "" && request.get_host() == Servers[i].get_server_name()))
				server = Servers[i];
		}
	}
	return server;
}

Location *Response::get_location(Request request, Server &server)
{
	size_t current_location_size = 0;
	int stored_index = -1;
	int current_index = 0;
	size_t dotpos;
	for(vector<Location>::iterator it = server.get_location().begin(); it != server.get_location().end(); it++)
	{
		if(it->get_path() == request.get_target())
			return(&(*it));
		else if(it->get_path().find(".") != std::string::npos && (dotpos = request.get_target().find_last_of(".")) != std::string::npos)
		{
			//std::cout << "location path: " << it->get_path() << std::endl;
			//std::cout << "request path: " << request.get_target() << std::endl;
			string extension_part = request.get_target().substr(dotpos, request.get_target().length() - dotpos);
			if(extension_part.find("/") == string::npos && extension_part == it->get_path())
			{
				//std::cout << "entered return block" << std::endl;
				return(&(*it));
			}
		}
		else if(it->get_path() == request.get_target().substr(0, it->get_path().length()) && it->get_path().size() > current_location_size)
			stored_index = current_index;
		current_index++;
	}
	// std::cout << "request path: " << request.get_target() << std::endl;
	// std::cout << "location path: " << server.get_location()[stored_index].get_path() << std::endl;
	if(stored_index > -1)
		return (&(server.get_location()[stored_index]));
	return NULL;
}

string Response::get_full_resource_path(Request request, Location &location)
{
	string encoded_input = request.get_target();
	string decoded_input = urlDecode(encoded_input);
	string res;

	res += "."; // need this if not its gonna be considered absolute path bruh
	if(location.get_root() != "")
		res += location.get_root();
	res += decoded_input;
	return res;
}

int		Response::check_allowed_methods(string method, Location &location)
{
	for(vector<string>::iterator it = location.get_allowed_methods().begin(); it != location.get_allowed_methods().end(); it++)
	{
		if(*it == method)
			return 1;
	}
	return 0;
}

void	Response::handle_return(Request request, Server &server, Location &location)
{
	string return_ = location.get_return();
	size_t pos = 0;
	string status_code;
	size_t temp;
	while(return_[pos] == ' ' || return_[pos] == '\t')
		pos++;
	if(std::isdigit(return_[pos]))
	{
		temp = return_.find_first_of(" \t");
		if(temp == string::npos)
			throw std::runtime_error("Error: invalid return part");
		status_code = return_.substr(pos, temp - pos);
		pos = temp + 1;
	}
	else
		status_code = "302";
	while(return_[pos] == ' ' || return_[pos] == '\t')
		pos++;
	string path = return_.substr(pos, return_.size() - pos);

	this->response_data += get_start_line(request, status_code, server);
	this->response_data += "Location: " + path + "\r\n" + get_headers("", "text/html");
	//surprisingly this one dn code the website can just change the location to the path then it will move u there liao wth
	//std::cout << "finish running handle return" << std::endl;
}

int	Response::check_request_body_valid(Request &request)
{
	if(request.get_content_length().length() == 0)
		return 1;
	string req = request.get_data();
	size_t first_boundary_start = req.find("\r\n\r\n") + 4;
	if(first_boundary_start >= req.length())
		return 0;
	size_t boundary_delim = req.find_first_of("\n", first_boundary_start);
	string boundary = req.substr(first_boundary_start, boundary_delim - first_boundary_start);
	size_t find_second_boundary = req.find(boundary);
	if(find_second_boundary == string::npos)
		return 0;
	return 1;
}