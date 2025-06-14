#include "Cgi.hpp"
#include "Socket.hpp"
#include "Location.hpp"
#include <map>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>

#ifdef __APPLE__
	#include <csignal>
	#include <signal.h>
#endif
using std::string;
// using std::cout;
// using std::endl;
using std::map;

Cgi::Cgi() {}

Cgi::~Cgi() {}

static std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

// I love Deepseek AI
int waitpid_with_timeout(pid_t pid, unsigned int seconds) {
	int sel = 1;
	int selectPid = fork();
	int pidStatus = 0;
	int selectStatus = 0;
	if(selectPid == 0) {
		fd_set readfds;
		struct timeval timeout;

		FD_ZERO(&readfds);
        // We need some fd to watch - using stdin (0) as dummy
        FD_SET(0, &readfds);
        
        timeout.tv_sec = seconds;
        timeout.tv_usec = 0;
        sel = select(1, &readfds, NULL, NULL, &timeout);
		if (sel == 0) exit(2);  // Timeout
        if (sel == -1) exit(1); // Error
		exit(0);
	}
    
    while (true) {
        int result = waitpid(pid, &pidStatus, WNOHANG);
		int resultSelect = waitpid(selectPid, &selectStatus, WNOHANG);
        
        if (result == pid) return pid;
        if (result == -1) return -1;
		if (resultSelect == -1) {
			if(WIFEXITED(selectStatus) && WEXITSTATUS(selectStatus) == 2) {
				return -2;
			}
			return -1;
		}
    }
}

char	**Cgi::config_env(Request &request)
{
	char **env = (char **)malloc(sizeof(char *) * request.get_header_fields().size() * 5);

	env[0] = strdup((string("REQUEST_METHOD=") + request.get_method()).c_str());
	env[1] = strdup((string("ROUTE=") + request.get_target()).c_str());
	env[2] = strdup((string("SERVER_PROTOCOL=") + request.get_http_ver()).c_str());
	size_t j = 3;

	for(map<string, string>::iterator it = request.get_header_fields().begin(); it != request.get_header_fields().end(); it++)
	{
		string field = it->first;
		// Not available in c++98
		// std::replace(field.begin(), field.end(), '-', '_');
		field = replaceAll(field, "-", "_");
		for(size_t i = 0; i < field.size(); i++)
			field[i] = std::toupper(field[i]);
		env[j++] = strdup((field + "=" + it->second).c_str());
		setenv(field.c_str(), it->second.c_str(), 1); //1 means overwrite existing ones
	}
	env[j] = NULL;
	return env;
}

string	Cgi::get_cgi_output(int pipefd)
{
	char buffer[10000];
	string res;
	int bytesread;
	memset(buffer, 0, 10000);
	while((bytesread = read(pipefd, buffer, 10000)) > 0)
		res.append(buffer, bytesread);
	return res;
}

int Cgi::check_valid_response(std::string str)
{
	if(str.substr(0, 8) != "HTTP/1.1")
		return -1;
	return 1;
}


int 	Cgi::check_cgi_hang(Request &request, Response &response, Location &location, Server &server)
{
	int pid = fork();
	int exit_status;
	(void)server;
	if(pid == 0)
	{
		string cgi_path = "." + location.get_cgi_pass();
		pid_t pid;
		int pipefd_input[2];
		int pipefd_output[2];
		int exit_status;
		//for normal scripts
		char *argv[] = {(char *)cgi_path.c_str(), NULL}; // <- undo this if wanna do normal script

		//for python scripts only
		// string pythoninterprator = "/usr/bin/python3";
		// char *argv[] = {(char *)pythoninterprator.c_str(), (char*)cgi_path.c_str(), NULL};

		//cout << "REQUEST BODY: " << request.get_body() << "\n----------------------------------" << endl;
		if(pipe(pipefd_input) == -1 || pipe(pipefd_output) == -1)
			exit(1);
		else
		{
			pid = fork();
			if(pid == 0) //child process
			{
				close(pipefd_input[0]);
				close(pipefd_output[0]);
				close(pipefd_output[1]);
				if(write(pipefd_input[1], request.get_body().c_str(), request.get_body().size()) == -1)
					throw std::runtime_error("Error: cgi write failed");
				//close(pipefd_input[1]);
				exit(0);
			}
			// waitpid(pid, &exit_status,0); // If send large file, the write func in child will block
			pid = fork();
			if(pid == 0)
			{
				dup2(pipefd_input[0], STDIN_FILENO);
				dup2(pipefd_output[1], STDOUT_FILENO);
				close(pipefd_input[0]);
				close(pipefd_input[1]);
				// close(pipefd_output[0]);
				close(pipefd_output[1]);
				char **env = this->config_env(request);
				//std::cerr << "reached here" << std::endl;
				execve(cgi_path.c_str(), argv, env);
				perror("execve");
				exit(-1);
			}
			close(pipefd_input[0]);
			close(pipefd_input[1]);
			close(pipefd_output[1]);
			string cgi_output = get_cgi_output(pipefd_output[0]);
			int result = waitpid(pid, &exit_status, 0);
			if(result <= 0)
			{
				// std::cout << result << std::endl;
				if (result == -2) {
					// You might want to kill the child here
					kill(pid, SIGKILL);
					waitpid(pid, &exit_status, 0);  // Clean up zombie
				}
				exit(1);
			}
			else
				response.set_response(cgi_output);
			close(pipefd_output[0]);
			exit(0);
		}
	}
	else
	{	
		int result = waitpid_with_timeout(pid, 5);
		if(result <= 0)
		{
			//std::cout << result << std::endl;
			if (result == -2) {
				// You might want to kill the child here
				kill(pid, SIGKILL);
				waitpid(pid, &exit_status, 0);  // Clean up zombie
			}
			return -1;
		}
		return 1;
	}
}


void	Cgi::Cgi_main(Request &request, Response &response, Location &location, Server &server)
{
	string cgi_path = "." + location.get_cgi_pass();
	pid_t pid;
	int pipefd_input[2];
	int pipefd_output[2];
	int exit_status;
	//for normal scripts
	char *argv[] = {(char *)cgi_path.c_str(), NULL}; // <- undo this if wanna do normal script

	//for python scripts only
	// string pythoninterprator = "/usr/bin/python3";
	// char *argv[] = {(char *)pythoninterprator.c_str(), (char*)cgi_path.c_str(), NULL};

	//cout << "REQUEST BODY: " << request.get_body() << "\n----------------------------------" << endl;
	if(check_cgi_hang(request, response, location, server) == -1)
		response.handle_error(request, "500", server);
	else if(pipe(pipefd_input) == -1 || pipe(pipefd_output) == -1)
		response.handle_error(request, "500", server);
	else
	{
		pid = fork();
		if(pid == 0) //child process
		{
			close(pipefd_input[0]);
			close(pipefd_output[0]);
			close(pipefd_output[1]);
			if(write(pipefd_input[1], request.get_body().c_str(), request.get_body().size()) == -1)
				throw std::runtime_error("Error: cgi write failed");
			close(pipefd_input[1]);
			exit(0);
		}
		// waitpid(pid, &exit_status,0); // If send large file, the write func in child will block
		pid = fork();
		if(pid == 0)
		{
			dup2(pipefd_input[0], STDIN_FILENO);
			dup2(pipefd_output[1], STDOUT_FILENO);
			close(pipefd_input[0]);
			close(pipefd_input[1]);
			close(pipefd_output[0]);
			close(pipefd_output[1]);
			char **env = this->config_env(request);

			//for python scripts
			execve(cgi_path.c_str(), argv, env);
			perror("execve");
			exit(-1);
		}
		close(pipefd_input[0]);
		close(pipefd_input[1]);
		close(pipefd_output[1]);
		string cgi_output = get_cgi_output(pipefd_output[0]);
		int result = waitpid(pid, &exit_status, 0);
		if(result <= 0)
		{
			// std::cout << result << std::endl;
			if (result == -2) {
				// You might want to kill the child here
				kill(pid, SIGKILL);
				waitpid(pid, &exit_status, 0);  // Clean up zombie
			}
			response.handle_error(request, "500", server);
		}
		else
		{
			if(check_valid_response(cgi_output) == 1)	
				response.set_response(cgi_output);
			else
				response.handle_error(request, "500", server);
		}
		close(pipefd_output[0]);
	}
}