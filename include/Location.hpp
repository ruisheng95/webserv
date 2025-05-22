#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include <vector>

class Location
{
	private:
		std::string			path; // the route outside the location brackets
		std::vector<std::string>	allowed_methods;
		std::string			root;
		std::vector<std::string>	index;
		std::string			cgi_pass;
		std::string			return_;
		bool			autoindex; //directory listing

	public:
		Location();
		~Location();

		void	skip_whitespaces(size_t &pos, std::string file_data);
		void	print_location();
	
		//get
		std::vector<std::string> &get_allowed_methods();
		std::string		get_return();
		std::string		get_root();
		std::string		get_path();
		std::vector<std::string>	&get_index();
		bool		get_autoindex();
		std::string		get_cgi_pass();


		//parse
		void	parse_location_main(size_t &pos, std::string file_data);
		void	parse_allowed_methods(size_t &pos, std::string file_data);
		void	parse_root(size_t &pos, std::string file_data);
		void	parse_index(size_t &pos, std::string file_data);
		void	parse_cgi_pass(size_t &pos, std::string file_data);
		void	parse_return(size_t &pos, std::string file_data);
		void	parse_autoindex(size_t &pos, std::string file_data);
};

#endif