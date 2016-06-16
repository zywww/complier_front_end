#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class Tag
{
	ADD,					// +
	SUB,					// -
	MUL,					// *
	DIV,					// /
	NOT,					// !
	NE,						// !=
	LT,						// < 
	LE,						// <=
	GT,						// >
	GE,						// >=
	ASSIGN,					// =
	EQ,						// ==
	OR,						// ||
	AND,					// &&
	LEFT_BRACES,			// {
	RIGHT_BRACES,			// }
	SEMICOLON,				// ;
	LEFT_BRACKETS,			// [
	RIGHT_BRACKETS,			// ]
	LEFT_PAREN,				// (
	RIGHT_PAREN,			// )

	IF,			
	ELSE,
	WHILE,
	DO,
	BREAK,
	TRUE,
	FALSE,

	INT,		// type
	FLOAT,		// type
	BOOL,		// type

	ID,			// 标识符
	INTEGER,	// 整数字面值
	REAL,		// 浮点数字面值

	TEMP,
	INDEX,

	UNKNOW,
	END_OF_FILE
};

struct Token
{
	Tag tag;

	explicit Token(Tag t) : tag(t) { }

	static bool numeric(Token type);
	static Token max(Token lhs, Token rhs);

	virtual std::string toString();
};

struct ID : public Token
{
	//friend bool operator==(const ID &lhs, const ID &rhs);
	//friend bool operator!=(const ID &lhs, const ID &rhs);

	std::string lexeme;

	explicit ID(std::string l) : Token(Tag::ID), lexeme(l) { }

	virtual std::string toString();
};

struct Integer : public Token
{
	int value;

	explicit Integer(int v) : Token(Tag::INTEGER), value(v) { }

	virtual std::string toString();
};

struct Real : public Token
{
	double value;

	explicit Real(double v) : Token(Tag::REAL), value(v) { }

	virtual std::string toString();
};

struct Type
{	
	static Type *typeint;
	static Type *typefloat;
	static Type *typebool;
	Tag type;
	size_t width = 0;

	Type(Tag tp, size_t wd);

	static bool numeric(Type *p);
	static Type *max(Type *p1, Type *p2);
};

struct Array : public Type
{
	Type *type;	
	size_t	size;

	Array(Type *t, size_t sz);
};

#endif
