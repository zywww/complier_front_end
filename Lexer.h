#ifndef LEXER_H
#define LEXER_H

#include <fstream>
#include <map>
#include "Token.h"

enum class State
{
	START,
	SYMBOL,
	ASIGNMENT,
	EQUAL,
	WAIT_OR,
	OR,
	WAIT_AND,
	AND,
	NOT,
	NOT_EQUAL,
	LESS_THAN,
	LESS_EQUAL,
	GRATER_THAN,
	GRATER_EQUAL,
	INTEGER,
	FLOAT,
	IDENTIFIER
};

class Lexer
{
	std::ifstream	fin;				// 文件流
	std::string		code;				// 词法分析器分析的代码文本
	std::string		buffer;				// 词法单元缓存
	size_t			curr = 0;			// 代码文本下标
	

	void error();						// 输出词法错误和错误所在的行号
	void skipWhiteSpace();

	static std::map<std::string, Tag> keywords;		

	static void preprocessor(std::string &str);			// 预处理，去掉注释
	
public:
	static size_t lineNumber;			// 当前行号

	Lexer(const std::string &filename);

	Token *getNextToken();
	size_t getLineNumber();
};

#endif