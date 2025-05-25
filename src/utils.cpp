#include <sstream>
#include <sys/types.h>
#include <iostream>

ssize_t my_atoi(std::string s) {
	ssize_t i;
	std::istringstream(s) >> i;
	return i;
}

std::string my_itoa(size_t i)
{
	std::string s;
	std::stringstream out;
	out << i;
	s = out.str();
	return s;
}

int my_hex_to_int(std::string s) {
	int i;
	std::stringstream ss;
	ss << std::hex << s;
	ss >> i;
	return i;
}

void	print_2d_array(char **cmd)
{
	int	i;

	i = 0;
	while (cmd[i])
	{
		std::cout << cmd[i] << std::endl; 
		i++;
	}
}

// #include <iostream>
// int main()
// {
//     std::cout << my_atoi("2147483999");
// }