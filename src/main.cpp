#include <iostream>
#include <string>
#include <tree_sitter/api.h>

extern "C" TSLanguage *tree_sitter_cpp();

int main()
{
	TSParser *parser = ts_parser_new();
	std::cout << "test" << std::endl;
	return 0;
}
