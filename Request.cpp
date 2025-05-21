#include "Request.hpp"
#include "Socket.hpp"
#include <fcntl.h>
#include <string>
#include <sys/socket.h>
#include <iostream>
using std::string;
using std::map;
using std::pair;
using std::cout;
using std::endl;

ssize_t my_atoi(std::string s);

Request::Request() {}

Request::~Request() {}

//get

string	Request::get_req_data()
{
	return this->req_data;
}

string	Request::get_method()
{
	return this->method;
}

string	Request::get_target()
{
	return this->target;
}

string	Request::get_http_ver()
{
	return this->http_ver;
}

string	Request::get_host()
{
	return this->host;
}

string	Request::get_port()
{
	return this->port;
}

string	Request::get_content_length()
{
	return this->content_length;
}

map<string, string>	&Request::get_header_fields()
{
	return this->header_fields;
}

string	Request::get_body()
{
	return this->body;
}

string Request::get_data()
{
	return this->req_data;
}

void Request::set_data(string newdata)
{
	this->req_data = newdata;
}

void Request::parse_request_line(size_t &pos)
{
	size_t temp;
	temp = req_data.find_first_of(" \t", pos);
	this->method = req_data.substr(pos, temp - pos);
	pos = temp + 1;
	temp = req_data.find_first_of(" \t", pos);
	this->target = req_data.substr(pos, temp - pos);
	pos = temp + 1;
	temp = req_data.find_first_of("\r", pos);
	this->http_ver = req_data.substr(pos, temp - pos);
	pos = temp + 2;
	// if(method.length() == 0 || target.length() == 0 || http_ver.length() == 0)
	// 	throw CustomException("Error: Request error");
}

void Request::parse_host_port(string str)
{
	//extract host
	size_t temp;
	temp = str.find_first_of(":", 0);
	this->host = str.substr(0, temp);

	//extract port
	this->port = str.substr(temp + 1, str.length() - temp);
}


void Request::parse_header_fields(size_t &pos)
{
	size_t temp;
	pair <string, string> pair;
	while(pos < req_data.length() && req_data[pos] != '\r') // second condition is to check for last line
	{
		temp = req_data.find_first_of(":" , pos);
		pair.first = req_data.substr(pos, temp - pos);
		pos = temp + 2;
		temp = req_data.find_first_of("\r", pos); //mengseong used /n but i thot the header fields end wif /r/n
		pair.second = req_data.substr(pos, temp - pos);
		pos = temp + 2;
		if(pair.first == "Host")
			parse_host_port(pair.second);
		if(pair.first == "Content-Length")
			this->content_length = pair.second;
		this->header_fields.insert(pair);
	}
}

void Request::parse_body(size_t &pos, int socket_fd)
{
	//for rng laptop (idk if it will work lol i cant test): 
	pos += 2; // skip /r/n

	if(this->content_length.length() != 0)
		this->body = req_data.substr(pos, req_data.length() - pos); //get the rest of the data ig
	(void)socket_fd;

	//get the rest of the body for big files
	// int expected_len = my_atoi(this->content_length.c_str());
	// int curr_len = this->body.size();
	// char buffer[1000];
	// int bytes_to_read;
	// int bytesrecv;
	// while(curr_len < expected_len)
	// {
	// 	//cout << "curr:" << curr_len << "expected:" << expected_len << endl;
	// 	if(expected_len - curr_len > 1000)
	// 		bytes_to_read = 1000;
	// 	else
	// 		bytes_to_read = expected_len - curr_len;
	// 	bytesrecv = recv(socket_fd, buffer, bytes_to_read, 0);
	// 	buffer[bytesrecv] = '\0';
	// 	this->body.append(buffer, bytesrecv);
	// 	this->req_data.append(buffer, bytesrecv); //must use append cannot use += if not the null terminator will corrupt the file data
	// 	curr_len += bytesrecv;
	// }
	//cout << this->body << endl;
}

void Request::parse_request_data_main(int socket_fd)
{
	size_t pos = 0;
	//cout << this->req_data << endl;
	parse_request_line(pos);
	parse_header_fields(pos);
	parse_body(pos, socket_fd);
	//print_request(*this);
}

void	print_request(Request request)
{
	cout << "REQUEST ATTRIBUTES" << endl;
	cout << "==================" << endl;
	cout << "method: " << request.get_method() << endl;
	cout << "route: " << request.get_target() << endl;
	cout << "http_protocol: " << request.get_http_ver() << endl;
	cout << "host: " << request.get_host() << endl;
	cout << "port: " << request.get_port() << endl;
	cout << "content_length: " << request.get_content_length() << endl;
	map<string, string> header_fields = request.get_header_fields();
	map<string, string>::iterator it = header_fields.begin();
	map<string, string>::iterator ite = header_fields.end();
	cout << endl;
	cout << "Header fields" << endl;
	cout << "-------------" << endl;
	for (; it != ite; it++)
	{
		cout << it->first << ":" << it->second << endl;
	}
	cout << "-------------" << endl;
	cout << "Message Body" << endl;
	cout << request.get_body() << endl;
	cout << "==================" << endl;
}