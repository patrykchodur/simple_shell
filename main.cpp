#include <iostream>

#include "parser.hpp"

int main(int argc, char* argv[]) {
	Parser parser(argc, argv);
	parser.main_loop();

}
