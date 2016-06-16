#include <sstream>
#include "Token.h"

bool Token::numeric(Token type)
{
	return true;
}
Token Token::max(Token lhs, Token rhs)
{
	if (lhs.tag == Tag::FLOAT || rhs.tag == Tag::FLOAT)
		return Token(Tag::FLOAT);
	else if (lhs.tag == Tag::INT || rhs.tag == Tag::INT)
		return Token(Tag::INT);
	else
		return Token(Tag::BOOL);
}
std::string Token::toString()
{
	switch (tag)
	{
	case Tag::ADD:	  return "+";
	case Tag::SUB:	  return "-";
	case Tag::MUL:	  return "*";
	case Tag::DIV:	  return "/";
	case Tag::NOT:	  return "!";
	case Tag::NE:	  return "!=";
	case Tag::LT:	  return "<";
	case Tag::LE:	  return "<=";
	case Tag::GT:	  return ">";
	case Tag::GE:     return ">=";
	case Tag::ASSIGN: return "=";
	case Tag::EQ:	  return "==";
	case Tag::OR:	  return "||";
	case Tag::AND:	  return "&&";
	case Tag::TRUE:	  return "true";
	case Tag::FALSE:  return "false";
	default:	      return "";
	}
}

std::string ID::toString()
{
	return lexeme;
}

std::string Integer::toString()
{
	std::ostringstream os;
	os << value;
	return os.str();
}

std::string Real::toString()
{
	std::ostringstream os;
	os << value;
	return os.str();
}

Type *Type::typeint = new Type(Tag::INT, 4);
Type *Type::typefloat = new Type(Tag::FLOAT, 8);
Type *Type::typebool = new Type(Tag::BOOL, 1);

Type::Type(Tag tp, size_t wd) : type(tp), width(wd) { }
bool Type::numeric(Type *p)
{
	switch (p->type)
	{
	case Tag::INT:
	case Tag::FLOAT:
	case Tag::BOOL:
		return true;
	default:
		return false;
	}
}
Type* Type::max(Type *p1, Type *p2)
{
	if (!numeric(p1) || !numeric(p2)) return nullptr;
	else if (p1->type == Tag::FLOAT || p2->type == Tag::FLOAT) return Type::typefloat;
	else return Type::typeint;
}

Array::Array(Type *t, size_t sz) : Type(Tag::INDEX, sz*t->width), type(t), size(sz) { }
