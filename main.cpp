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

// TODO 命名空间
 
int main()
{
	test("test.txt");
	
	cout << "由 VS 提交到 GitHub" << endl;

	system("pause");
	return 0;
}