#include <string>
#include <iostream>
#include <algorithm>
#include "LLParser.h"

using std::cout;
using std::endl;
using std::string;

std::vector<Tag> LLParser::follow_stmt{ Tag::ELSE, Tag::WHILE, Tag::INT, Tag::BOOL, Tag::FLOAT,
	Tag::ID, Tag::IF, Tag::DO, Tag::BREAK, Tag::LEFT_BRACES, Tag::SEMICOLON };
std::vector<Tag> LLParser::follow_type{ Tag::ID, Tag::LEFT_BRACKETS };
std::vector<Tag> LLParser::follow_loc{ Tag::LEFT_BRACKETS, Tag::ASSIGN, Tag::ADD, Tag::SUB, Tag::MUL, 
Tag::DIV, Tag::LT, Tag::LE, Tag::GT, Tag::GE, Tag::EQ, Tag::NE, Tag::RIGHT_PAREN, Tag::SEMICOLON };
std::vector<Tag> LLParser::follow_boolable{ Tag::RIGHT_BRACES, Tag::RIGHT_BRACKETS, Tag::OR, Tag::SEMICOLON };
std::vector<Tag> LLParser::follow_join{ Tag::RIGHT_BRACES, Tag::RIGHT_BRACKETS, Tag::OR, Tag::SEMICOLON };
std::vector<Tag> LLParser::follow_equal{ Tag::RIGHT_BRACES, Tag::RIGHT_BRACKETS, Tag::OR, Tag::SEMICOLON, 
Tag::AND };
std::vector<Tag> LLParser::follow_rel{ Tag::RIGHT_BRACES, Tag::RIGHT_BRACKETS, Tag::OR, Tag::SEMICOLON, 
Tag::AND, Tag::EQ, Tag::NE };
std::vector<Tag> LLParser::follow_expr{ Tag::RIGHT_BRACES, Tag::RIGHT_BRACKETS, Tag::OR, Tag::SEMICOLON, 
Tag::AND, Tag::EQ, Tag::NE, Tag::LE, Tag::LT, Tag::GE, Tag::GT };
std::vector<Tag> LLParser::follow_term{ Tag::RIGHT_BRACES, Tag::RIGHT_BRACKETS, Tag::OR, Tag::SEMICOLON, 
Tag::AND, Tag::EQ, Tag::NE, Tag::LE, Tag::LT, Tag::GE, Tag::GT, Tag::ADD, Tag::SUB };
std::vector<Tag> LLParser::follow_unary{ Tag::RIGHT_BRACES, Tag::RIGHT_BRACKETS, Tag::OR, Tag::SEMICOLON, 
Tag::AND, Tag::EQ, Tag::NE, Tag::LE, Tag::LT, Tag::GE, Tag::GT, Tag::ADD, Tag::SUB };
std::vector<Tag> LLParser::follow_factor{ Tag::RIGHT_BRACES, Tag::RIGHT_BRACKETS, Tag::OR, Tag::SEMICOLON, 
Tag::AND, Tag::EQ, Tag::NE, Tag::LE, Tag::LT, Tag::GE, Tag::GT, Tag::ADD, Tag::SUB };

bool Env::put(string var, ASTid *id)
{
	bool result = varTable.insert({ var, id }).second;
	return result;
}
ASTid* Env::get(string var)
{
	for (Env *e = this; e != nullptr; e = e->prev)
	{
		auto result = e->varTable.find(var);
		if (result == e->varTable.end())
			continue;
		else
			return result->second;
	}
	return nullptr;
}

LLParser::LLParser(string filename) : lex(filename) 
{
	getNextToken();
	top = new Env(nullptr);
}

bool LLParser::matchToken(Tag tag)
{
	if (token->tag == tag)
		return true;
	else
		return false;
}
void LLParser::getNextToken()
{
	token = lex.getNextToken();
}
void LLParser::skipUntil(const std::vector<Tag> &follow)
{
	while (!(matchToken(Tag::END_OF_FILE) || 
		std::find(follow.begin(), follow.end(), token->tag) != follow.end()))
		getNextToken();
}
void LLParser::error(const std::string info)
{
	error_happen = true;
	cout << "�� " << lex.getLineNumber() << " �и����﷨����" << info << endl;
}

void LLParser::parse()
{
	ASTstmt *s = stmts();
	if (error_happen || ASTnode::wrong_happen)
	{
		cout << "�﷨��������" << endl;
	}
	else
	{
		cout << "�﷨�����ɹ�" << endl;
		cout << "�����м����" << endl;
		if (s)
		{
			size_t	begin = s->newLabel();
			size_t	after = s->newLabel();
			s->emitLabel(begin);
			s->gen(begin, after);
			s->emitLabel(after);
		}
		else
		{
			cout << "����Ϊ��" << endl;
		}
	}
}
ASTstmt* LLParser::stmts()
{
	if (matchToken(Tag::END_OF_FILE)) return nullptr;
	else
	{
		ASTstmt *s1 = stmt();
		return new ASTseq(s1, stmts());
	}
}
ASTstmt* LLParser::stmt()
{
	ASTexpr *ex;
	ASTstmt *s, *s1, *s2, *saved;
	ASTwhile *sw = nullptr;
	ASTdo *sdo = nullptr;
	switch (token->tag)
	{
	case Tag::BOOL:
	case Tag::INT:
	case Tag::FLOAT: decl(); return nullptr; 
	case Tag::ID: return assign(); 
	case Tag::IF:
		// �˴������� else ���⣬��Ϊÿ�ζ�������� else ��Ϊ if ��Ӧ�����
		getNextToken(); 
		if (matchToken(Tag::LEFT_PAREN))
			getNextToken();
		else
			error("ȱ�� '('");
		ex = boolable(); 
		if (matchToken(Tag::RIGHT_PAREN))
			getNextToken();
		else 
			error("ȱ�� ')'");
		s1 = stmt();
		if (matchToken(Tag::ELSE))
		{
			getNextToken();
			s2 = stmt();
			return new ASTifelse(ex, s1, s2);
		}
		else
		{
			return new ASTif(ex, s1);
		}
		break;
	case Tag::WHILE: 
		saved = ASTstmt::currWhile;
		sw = new ASTwhile();
		ASTstmt::currWhile = sw;
		getNextToken();
		if (matchToken(Tag::LEFT_PAREN))
			getNextToken();
		else 
			error("ȱ�� '('");
		ex = boolable();
		if (matchToken(Tag::RIGHT_PAREN))
			getNextToken();
		else 
			error("ȱ�� ')'");
		s = stmt();
		sw->init(ex, s);
		ASTstmt::currWhile = saved;
		return sw;
	case Tag::DO: 
		saved = ASTstmt::currWhile;
		sdo = new ASTdo();
		ASTstmt::currWhile = sdo;
		getNextToken();
		s = stmt();
		if (!matchToken(Tag::WHILE))
		{
			error("ȱ�� while");
			skipUntil(LLParser::follow_stmt);
			return nullptr;
		}
		else
		{
			getNextToken();
			if (matchToken(Tag::LEFT_PAREN))
				getNextToken();
			else
				error("ȱ�� '('");
				
			ex = boolable();
			if (matchToken(Tag::RIGHT_PAREN))
				getNextToken();
			else
				error("ȱ�� ')'");
			if (matchToken(Tag::SEMICOLON))
				getNextToken();
			else
				error("ȱ�� ';'");
		}
		sdo->init(ex, s);
		ASTstmt::currWhile = saved;
		return sdo;
		break;
	case Tag::BREAK: 
		getNextToken(); 
		if (matchToken(Tag::SEMICOLON))
			getNextToken();
		else
			error("ȱ�� ';'");
		return new ASTbreak();
	case Tag::LEFT_BRACES: 
		return block(); 
		break;
	case Tag::SEMICOLON: 
		getNextToken(); 
		return nullptr;
		break;
	default: error("������"); skipUntil(LLParser::follow_stmt); 
	}
	return nullptr;
}
ASTstmt* LLParser::assign()
{
	ASTid *id;
	
	id = top->get(token->toString());
	if (!id)
	{
		error("���� " + token->toString() + " δ����");
		skipUntil(LLParser::follow_loc);
		return nullptr;
	}
	else
	{
		getNextToken();
	}

	if (matchToken(Tag::ASSIGN))
	{
		getNextToken();
		ASTexpr *ex = boolable();
		if (matchToken(Tag::SEMICOLON))
		{
			getNextToken();
			return new ASTassign(id, ex);
		}
		else
		{
			error("ȱ�� ';'");
			return new ASTassign(id, ex);
		}
	}
	else if (matchToken(Tag::LEFT_BRACKETS))
	{
		ASTaccess *x = offset(id);
		if (matchToken(Tag::ASSIGN))
		{
			getNextToken();
		}
		else
		{
			error("ȱ�ٸ�ֵ��");
			skipUntil(LLParser::follow_stmt);
			return nullptr;
		}
		ASTexpr *ex = boolable();
		if (matchToken(Tag::SEMICOLON))
		{
			getNextToken();
			return new ASTsetelem(x, ex);
		}
		else
		{
			error("ȱ�� ';'");
			return new ASTsetelem(x, ex);
		}
	}
	else
	{
		error("ȱ�� '='");
		skipUntil(LLParser::follow_stmt);
		return nullptr;
	}
}
ASTaccess* LLParser::offset(ASTid *id)
{
	ASTexpr *index, *width, *temp1, *temp2, *loc;
	Type *type;

	type = id->type;
	getNextToken();
	index = boolable();
	if (matchToken(Tag::RIGHT_BRACKETS))
	{
		getNextToken();
	}
	else 
	{
		error("ȱ�� ']'");
		skipUntil(LLParser::follow_stmt);
		return nullptr;
	}
	type = static_cast<Array*>(type)->type;
	width = new ASTconstant(new Integer(type->width), Type::typeint);
	temp1 = new ASTarith(new Token(Tag::MUL), index, width);
	loc = temp1;

	while (matchToken(Tag::LEFT_BRACKETS))
	{
		getNextToken();
		index = boolable();
		if (matchToken(Tag::RIGHT_BRACKETS))
		{
			getNextToken();
		}
		else
		{
			error("ȱ�� ']'");
			skipUntil(LLParser::follow_loc);
			return nullptr;
		}
		type = static_cast<Array*>(type)->type;
		width = new ASTconstant(new Integer(type->width), Type::typeint);
		temp1 = new ASTarith(new Token(Tag::MUL), index, temp1);
		temp2 = new ASTarith(new Token(Tag::ADD), loc, temp1);
		loc = temp2;
	}
	return new ASTaccess(id, loc, type);
}
ASTstmt* LLParser::block()
{
	Env *saved = top;
	ASTstmt *s = nullptr;
	if (matchToken(Tag::LEFT_BRACES))
	{
		top = new Env(top);
		getNextToken();
	}
	else
	{
		error("ȱ�� '{'");
	}

	while (!(matchToken(Tag::RIGHT_BRACES) || matchToken(Tag::END_OF_FILE)))
	{
		s = new ASTseq(s, stmt());
	}
		

	if (matchToken(Tag::RIGHT_BRACES))
	{
		top = saved;
		getNextToken();
	}
	else
	{
		error("ȱ�� '}'");
	}
	return s;
}
void LLParser::decl()
{
	Type *type = nullptr;
	switch (token->tag)
	{
	case Tag::INT:		type = new Type(Tag::INT, 4); getNextToken(); break;
	case Tag::FLOAT:	type = new Type(Tag::FLOAT, 8); getNextToken(); break;
	case Tag::BOOL:		type = new Type(Tag::BOOL, 1);  getNextToken(); break;
	default: error("ȱ������");
	}
		
	if (matchToken(Tag::LEFT_BRACKETS))
	{
		type = arrayType(type);
		if (!type)
		{
			skipUntil(LLParser::follow_stmt);
			return;
		}
	}
		
	
	if (matchToken(Tag::ID))
	{
		ASTid *id= new ASTid(token, type, used);
		bool result = top->put(token->toString(), id);
		if (!result)
			error("�ظ����� " + token->toString());
		getNextToken();
	}
	else
	{
		error("ȱ�ٱ�����");
	}
	
	if (matchToken(Tag::SEMICOLON))
		getNextToken();
	else
		error("ȱ�ٷֺ�");
}
Type* LLParser::arrayType(Type *type)
{
	getNextToken();
	Integer *temp = nullptr;
	if (matchToken(Tag::INTEGER))
	{
		temp = static_cast<Integer*>(token);
		getNextToken();
	}
	else
	{
		error("�������飬�±����Ϊ������");
		return nullptr;
	}

	if (matchToken(Tag::RIGHT_BRACKETS))
		getNextToken();
	else
		error("ȱ�� ']'");

	if (matchToken(Tag::LEFT_BRACKETS))
	{
		type = arrayType(type);
	}


	if (temp && type)
		return new Array(type, temp->value);
	else
		return nullptr;
}
ASTexpr* LLParser::boolable()
{
	ASTexpr *ex = join();
	while (matchToken(Tag::OR))
	{
		getNextToken();
		ex = new ASTor(new Token(Tag::OR), ex, join());
	}
	return ex;
}
ASTexpr* LLParser::join()
{
	ASTexpr *ex = equality();
	while (matchToken(Tag::AND))
	{
		getNextToken();
		ex = new ASTand(new Token(Tag::AND), ex, equality());
	}
	return ex;
}
ASTexpr* LLParser::equality()
{
	ASTexpr *ex = rel();
	while (matchToken(Tag::EQ) || matchToken(Tag::NE))
	{
		Tag temp = token->tag;
		getNextToken();
		ex = new ASTrel(new Token(temp), ex, rel());
	}
	return ex;
}
ASTexpr* LLParser::rel()
{
	ASTexpr *ex =  expr();
	while (matchToken(Tag::LT) || matchToken(Tag::GT) || matchToken(Tag::LE) || matchToken(Tag::GE))
	{
		Tag temp = token->tag;
		getNextToken();
		ex = new ASTrel(new Token(temp), ex, expr());
	}
	return ex;
}
ASTexpr* LLParser::expr()
{
	ASTexpr *ex = term();
	while (matchToken(Tag::ADD) || matchToken(Tag::SUB))
	{
		
		Tag temp = token->tag;
		getNextToken();
		ex = new ASTarith(new Token(temp), ex, expr());
	}
	return ex;
}
ASTexpr* LLParser::term()
{
	ASTexpr *ex = unary();
	while (matchToken(Tag::MUL) || matchToken(Tag::DIV))
	{
		Tag temp = token->tag;
		getNextToken();
		ex = new ASTarith(new Token(temp), ex, unary());
	}
	return ex;
}
ASTexpr* LLParser::unary()
{
	if (matchToken(Tag::NOT) || matchToken(Tag::SUB))
	{
		if (matchToken(Tag::NOT))
		{
			getNextToken();
			return new ASTnot(new Token(Tag::NOT), factor());
		}
		else
		{
			getNextToken();
			return new ASTunary(new Token(Tag::SUB), factor());
		}
	}
	else
	{
		return factor();
	}
}
ASTexpr* LLParser::factor()
{
	if (matchToken(Tag::LEFT_PAREN))
	{
		getNextToken();
		ASTexpr *ex = boolable();
		if (matchToken(Tag::RIGHT_PAREN))
			getNextToken();
		else
			error("ȱ��������");
		return ex;
	}
	else if (matchToken(Tag::ID))
	{
		Token *temp = token;
		ASTid * id = top->get(token->toString());
		if (!id)
		{
			error("���� " + token->toString() + " δ����");
			skipUntil(LLParser::follow_factor);
			return nullptr;
		}
		getNextToken();
		if (matchToken(Tag::LEFT_BRACKETS))
			return offset(id);
		else
			return id;
	}
	else if (matchToken(Tag::INTEGER) || matchToken(Tag::REAL) || 
		matchToken(Tag::TRUE) || matchToken(Tag::FALSE))
	{
		if (matchToken(Tag::INTEGER))
		{
			Token *temp = token;
			getNextToken();
			return new ASTconstant(temp, Type::typeint);
		}
		else if (matchToken(Tag::REAL))
		{
			Token *temp = token;
			getNextToken();
			return new ASTconstant(temp, Type::typefloat);
		}
		else if (matchToken(Tag::TRUE))
		{
			getNextToken();
			return ASTconstant::booltrue;
		}
		else
		{
			getNextToken();
			return ASTconstant::boolfalse;
		}
	}
	else
	{
		error("ȱ����������");
		skipUntil(LLParser::follow_factor);
		return nullptr;
	}
}