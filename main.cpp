#include <iostream>

#include "parser.hpp"

int main(int argc, char* argv[]) {
	Parser parser(argc == 1);
	parser.main_loop();

}
