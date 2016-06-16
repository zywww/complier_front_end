#include <iostream>
#include <cctype>
#include "Lexer.h"

using std::string;
using std::cout;
using std::endl;

size_t Lexer::lineNumber = 1;
std::map<std::string, Tag> Lexer::keywords{ 
	{"if",    Tag::IF   },	{"else", Tag::ELSE},
	{"while", Tag::WHILE},  {"do",   Tag::DO  },
	{"break", Tag::BREAK},  {"true", Tag::TRUE},
	{"false", Tag::FALSE},  {"int",  Tag::INT },
	{"float", Tag::FLOAT},  {"bool", Tag::BOOL}
};

Lexer::Lexer(const std::string &s) :
	fin(s), code((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>())
{
	fin.close();
	preprocessor(code);
	cout << code << endl;

}

void Lexer::error()
{
	cout << "第 " << lineNumber << " 行词法错误 : 错误的符号 '" << code[curr] << "'" << endl;
}
//bool Lexer::isAcceptState(State state)
//{
//	switch (state)
//	{
//	case State::SYMBOL:
//	case State::ASIGNMENT:
//	case State::EQUAL: 
//	case State::OR: 
//	case State::AND:   
//	case State::NOT: 
//	case State::NOT_EQUAL:
//	case State::LESS_THAN:
//	case State::LESS_EQUAL:
//	case State::GRATER_THAN:
//	case State::GRATER_EQUAL:
//	case State::INTEGER:
//	case State::FLOAT:
//	case State::IDENTIFIER:
//		return true;
//	default:
//		return false;
//	}
//}
void Lexer::skipWhiteSpace()
{
	while (true)
	{
		if (code[curr] == ' ' || code[curr] == '\t')
			;	// 什么都不做
		else if (code[curr] == '\n')
			lineNumber++;
		else
			break;
		++curr;
	}
}

void Lexer::preprocessor(std::string &code)
{
	string temp;
	size_t index = 0;

	while (index < code.length())
	{
		switch (code[index])
		{
		case '/': if (code[index + 1] == '*')
		{
			index += 2;
			while ((code[index] != '*' || code[index + 1] != '/') && index < code.length())
			{
				if (code[index] == '\n')
					temp += '\n';
				++index;
			}
			++index;
		}
				  else if (code[index + 1] == '/')
				  {
					  index += 2;
					  while (code[index] != '\n' && index < code.length())
						  ++index;
					  if (code[index] == '\n')
						  temp += code[index];
				  }
				  else
				  {
					  temp += code[index];
				  }
				  index++;
				  break;
		default: temp += code[index]; index++;
		}
	}

	code = temp;
}

Token *Lexer::getNextToken()
{
	skipWhiteSpace();

	State state = State::START;
	while (curr < code.length())
	{
		switch (state)
		{
		case State::START:
			switch (code[curr])
			{
			case '{': ++curr; return new Token(Tag::LEFT_BRACES);
			case '}': ++curr; return new Token(Tag::RIGHT_BRACES);
			case ';': ++curr; return new Token(Tag::SEMICOLON);
			case '[': ++curr; return new Token(Tag::LEFT_BRACKETS);
			case ']': ++curr; return new Token(Tag::RIGHT_BRACKETS);
			case '(': ++curr; return new Token(Tag::LEFT_PAREN);
			case ')': ++curr; return new Token(Tag::RIGHT_PAREN);
			case '+': ++curr; return new Token(Tag::ADD);
			case '-': ++curr; return new Token(Tag::SUB);
			case '*': ++curr; return new Token(Tag::MUL);
			case '/': ++curr; return new Token(Tag::DIV);
				break;
			case '=': state = State::ASIGNMENT;	break;
			case '|': state = State::WAIT_OR; break;
			case '&': state = State::WAIT_AND; break;
			case '!': state = State::NOT; break;
			case '<': state = State::LESS_THAN; break;
			case '>': state = State::GRATER_THAN; break;
			default:
				if (std::isdigit(code[curr]))
				{
					buffer += code[curr];
					state = State::INTEGER;
				}
				else if (std::isalpha(code[curr]))
				{	
					buffer += code[curr];
					state = State::IDENTIFIER;
				}
				else
				{
					error();
					state = State::START;
				}
			}
			break;
//		case State::SYMBOL:
		case State::ASIGNMENT:
			switch (code[curr])
			{
			case '=': ++curr; return new Token(Tag::EQ);
			default : return new Token(Tag::ASSIGN);
			}
			break;
//		case State::EQUAL:
		case State::WAIT_OR:
			switch (code[curr])
			{
			case '|': ++curr; return new Token(Tag::OR);
			default:  error(); state = State::START;
			}
			break;
//		case State::OR:
		case State::WAIT_AND:
			switch (code[curr])
			{
			case '&': ++curr; return new Token(Tag::AND);
			default: error(); state = State::START;
			}
			break;
//		case State::AND:
		case State::NOT:
			switch (code[curr])
			{
			case '=': ++curr; return new Token(Tag::NE);
			default : return new Token(Tag::NOT);
			}
			break;
//		case State::NOT_EQUAL:
		case State::LESS_THAN:
			switch (code[curr])
			{
			case '=': ++curr; return new Token(Tag::LE);
			default : return new Token(Tag::LT);
			}
			break;
//		case State::LESS_EQUAL:
		case State::GRATER_THAN:
			switch (code[curr])
			{
			case '=': ++curr; return new Token(Tag::GE);
			default : return new Token(Tag::GT);
			}
			break;
//		case State::GRATER_EQUAL:
		case State::INTEGER:
			switch (code[curr])
			{
			case '.': buffer += code[curr]; state = State::FLOAT; break;
			default: 
				if (std::isdigit(code[curr]))
				{
					buffer += code[curr];
					state = State::INTEGER;
				}
				else
				{
					//++curr;
					int value = std::stoi(buffer);
					buffer = "";
					return new Integer(value);
				}
			}
			break;
		case State::FLOAT: 
			if (std::isdigit(code[curr]))
			{
				buffer += code[curr];
				state = State::FLOAT;
			}
			else
			{
				double value = std::stod(buffer);
				buffer = "";
				return new Real(value);
			}
			break;
		case State::IDENTIFIER:
			if (std::isdigit(code[curr]) || std::isalpha(code[curr]))
			{
				buffer += code[curr];
				state = State::IDENTIFIER;
			}
			else
			{
				auto result = keywords.find(buffer);
				if (result == keywords.end())
				{
					//++curr;
					string temp = buffer;
					buffer = "";
					return new ID(temp);
				}
				else
				{
					//++curr;
					buffer = "";
					return new Token(result->second);
				}
			}
			break;
		default:
			error(); state = State::START;
		}
		++curr;
	}

	return new Token(Tag::END_OF_FILE);
}
size_t Lexer::getLineNumber()
{
	return lineNumber;
}