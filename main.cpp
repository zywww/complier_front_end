#include <cstdlib>
#include <iostream>
#include "Lexer.h"
#include "LLParser.h"

using std::endl;
using std::cout;
using std::cin;

void test(const std::string &filename)
{
	LLParser parser(filename);
	parser.parse();
		
}



int main()
{
	test("test.txt");
	


	system("pause");
	return 0;
}