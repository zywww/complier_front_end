#ifndef LLPARSER_H
#define LLPARSER_H

#include <cassert>
#include <unordered_map>
#include <vector>
#include "AST.h"
#include "Lexer.h"

class Env
{
	std::unordered_map<std::string, ASTid*> varTable;
	Env *prev;
public:
	Env(Env *p) : prev(p) { }
	
	bool put(std::string var, ASTid *id);
	ASTid* get(std::string var);
};

class LLParser
{
private:
	// ͬ������
	static std::vector<Tag> follow_stmt;
	static std::vector<Tag> follow_type;
	static std::vector<Tag> follow_loc;
	static std::vector<Tag> follow_boolable;
	static std::vector<Tag> follow_join;
	static std::vector<Tag> follow_equal;
	static std::vector<Tag> follow_rel;
	static std::vector<Tag> follow_expr;
	static std::vector<Tag> follow_term;
	static std::vector<Tag> follow_unary;
	static std::vector<Tag> follow_factor;

	Lexer lex;					// �ʷ�������
	Token *token = nullptr;		// �Ӵʷ���������ȡ�Ĵʷ���Ԫ
	bool error_happen = false;	// �Ƿ����˴��������������������м����
	Env *top = nullptr;			// ������ű�
	size_t used = 0;			// ���ڱ��������Ĵ洢λ��

	void getNextToken();
	void skipUntil(const std::vector<Tag> &follow);		// �����ʷ���Ԫ��ֱ������ͬ�����ϻ��ļ�β�������﷨����ָ�
	bool matchToken(Tag tag);
	void error(const std::string info);					// ����﷨����ʹ��������к�

	// �﷨����
	ASTstmt* stmts();
	ASTstmt* stmt();
	ASTstmt* assign();
	ASTaccess* offset(ASTid *id);
	ASTstmt* block();
	void decl();
	Type* arrayType(Type *type);
	ASTexpr* boolable();
	ASTexpr* join();
	ASTexpr* equality();
	ASTexpr* rel();
	ASTexpr* expr();
	ASTexpr* term();
	ASTexpr* unary();
	ASTexpr* factor();

public:
	LLParser(std::string filename);

	void parse();
};

#endif