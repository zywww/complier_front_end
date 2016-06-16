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
	std::ifstream	fin;				// �ļ���
	std::string		code;				// �ʷ������������Ĵ����ı�
	std::string		buffer;				// �ʷ���Ԫ����
	size_t			curr = 0;			// �����ı��±�
	

	void error();						// ����ʷ�����ʹ������ڵ��к�
	void skipWhiteSpace();

	static std::map<std::string, Tag> keywords;		

	static void preprocessor(std::string &str);			// Ԥ����ȥ��ע��
	
public:
	static size_t lineNumber;			// ��ǰ�к�

	Lexer(const std::string &filename);

	Token *getNextToken();
	size_t getLineNumber();
};

#endif