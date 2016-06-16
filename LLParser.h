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
	// 同步集合
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

	Lexer lex;					// 词法分析器
	Token *token = nullptr;		// 从词法分析器获取的词法单元
	bool error_happen = false;	// 是否发生了错误，若发生错误，则不生成中间代码
	Env *top = nullptr;			// 顶层符号表
	size_t used = 0;			// 用于变量声明的存储位置

	void getNextToken();
	void skipUntil(const std::vector<Tag> &follow);		// 跳过词法单元，直到遇到同步集合或文件尾，用于语法错误恢复
	bool matchToken(Tag tag);
	void error(const std::string info);					// 输出语法错误和错误所在行号

	// 语法分析
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