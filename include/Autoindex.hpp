#ifndef AUTOINDEX_HPP
#define AUTOINDEX_HPP

#include <dirent.h>
#include <string>

class Autoindex
{
	public:
	std::string	get_file_size(size_t filesize);
	void	generate_autoindex_page_first_part(std::string &response_body, std::string dir_name);
	void	generate_autoindex_page_second_part(std::string &response_body, DIR *dir, std::string path);

	void	put_file_name(std::string &response_body, std::string filename, DIR *checkdir);
	void	put_last_modified_time(std::string &respons_body, struct stat &file_info);
	void	put_size(std::string &response_body, struct stat &file_info, DIR *checkdir);
	void	get_file_info(struct stat &file_info, std::string path_and_name);
};

#endif