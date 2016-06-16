#include <iostream>
#include <sstream>
#include <cassert>
#include "AST.h"

using std::string;
using std::endl;
using std::cout;


size_t ASTnode::label = 0;
bool ASTnode::wrong_happen = false;
ASTnode::ASTnode() : lexLine(Lexer::lineNumber) { }
size_t ASTnode::newLabel()
{
	return ++label;
}
void ASTnode::error(string info)
{
	wrong_happen = true;
	cout << "第 " << lexLine << " 行附近语义错误：" << info << endl;
}
void ASTnode::emitLabel(size_t lb)
{
	cout << "L" << lb << ": " ;
}
void ASTnode::emit(string str, size_t lb)
{
	if (lb == -1)
		cout << "\t" << str << endl;
	else 
		cout << "\t" << str << "L" << lb << endl;
}

ASTexpr::ASTexpr(Token *o, Type *tp) : ASTnode(), op(o), type(tp) { }
ASTexpr* ASTexpr::gen() { return this; }
ASTexpr* ASTexpr::reduce() { return this; }
void ASTexpr::jumping(size_t t, size_t f) { emitJump(toString(), t, f); }
void ASTexpr::emitJump(string test, size_t t, size_t f)
{
	if (t != 0 && f != 0)
	{
		emit("if " + test + " goto ", t);
		emit("goto ", f);
	}
	else if (t != 0) emit("if " + test + " goto ", t);
	else if (f != 0) emit("iffalse " + test + " goto ", f);
	else; // 不生成语句，因为 t 和 f 都直接穿越
}
string ASTexpr::toString() { return op->toString(); }

ASTconstant *ASTconstant::booltrue = new ASTconstant(new Token(Tag::TRUE), Type::typebool);
ASTconstant *ASTconstant::boolfalse = new ASTconstant(new Token(Tag::FALSE), Type::typebool);
ASTconstant::ASTconstant(Token *op, Type *tp) : ASTexpr(op, tp) { }
void ASTconstant::jumping(size_t t, size_t f)
{
	if (op->tag == Tag::TRUE && t != 0)
		emit("goto ", t);
	else if (op->tag == Tag::FALSE && f != 0)
		emit("goto ", f);
}	// 如果不是 bool 值的话，跳转怎么办

size_t ASTtemp::count = 0;
ASTtemp::ASTtemp(Type *tp) : ASTexpr(nullptr, tp), number(++count) { }
string ASTtemp::toString()
{
	std::ostringstream os;
	os << "t" << number;
	return os.str();
}

ASTid::ASTid(Token *id, Type *tp, size_t lc) : ASTexpr(id, tp), offset(lc) { }

ASTop::ASTop(Token *op, Type *tp) : ASTexpr(op, tp) { }
ASTexpr* ASTop::reduce()
{
	ASTexpr *ex = gen();
	ASTtemp *temp = new ASTtemp(type);
	emit(temp->toString() + " = " + ex->toString());
	return temp;
}

ASTaccess::ASTaccess(ASTid *a, ASTexpr *e, Type *p) : ASTop(nullptr, p), arr(a), index(e) { }
ASTexpr* ASTaccess::gen()
{
	return new ASTaccess(arr, index->reduce(), type);
}
string ASTaccess::toString()
{
	return arr->toString() + "[" + index->toString() + "]";
}

ASTarith::ASTarith(Token *op, ASTexpr *e1, ASTexpr *e2) : 
	ASTop(op, nullptr), expr1(e1), expr2(e2) 
{ 
	if (expr1->type && expr2->type)
	{
		type = Type::max(expr1->type, expr2->type);
		if (type == nullptr)
			error("类型错误");
	}
}
ASTexpr* ASTarith::gen()
{
	return new ASTarith(op, expr1->reduce(), expr2->reduce());
}
string ASTarith::toString()
{
	return expr1->toString() + " " + op->toString() + " " + expr2->toString();
}

ASTunary::ASTunary(Token *op, ASTexpr *e) : ASTop(op, nullptr), expr(e)
{
	type = Type::max(Type::typeint, expr->type);
}
ASTexpr* ASTunary::gen()
{
	return new ASTunary(op, expr->reduce());
}
string ASTunary::toString()
{
	return op->toString() + expr->toString();
}

ASTlogical::ASTlogical(Token *op, ASTexpr *e1, ASTexpr *e2) :
	ASTexpr(op, nullptr), expr1(e1), expr2(e2)
{
	type = check(expr1->type, expr2->type);
	if (type == nullptr)
		error("逻辑表达式因子为错误的类型");
}
Type* ASTlogical::check(Type *tp1, Type *tp2)
{
	if (typeid(tp1).name() == string("struct Array *") ||
		typeid(tp2).name() == string("struct Array *"))
		return nullptr;
	else if (tp1->type == tp2->type) return Type::typebool;
	else return nullptr;
}
ASTexpr* ASTlogical::gen()
{
	size_t f = newLabel();
	size_t a = newLabel();
	ASTtemp *temp = new ASTtemp(type);
	jumping(0, f);
	emit(temp->toString() + " = true");
	emit("goto ", a);
	emitLabel(f);
	emit(temp->toString() + " = false");
	emitLabel(a);

	return temp;
}
string ASTlogical::toString()
{
	return expr1->toString() + " " + op->toString() + " " + expr2->toString();
}

ASTand::ASTand(Token *op, ASTexpr *e1, ASTexpr *e2) : ASTlogical(op, e1, e2) { }
void ASTand::jumping(size_t t, size_t f)
{
	size_t label = f != 0 ? f: newLabel();
	expr1->jumping(0, label);
	expr2->jumping(t, f);
	if (f == 0) emitLabel(label);
}

ASTrel::ASTrel(Token *op, ASTexpr *e1, ASTexpr *e2) : ASTlogical(op, e1, e2) { }
void ASTrel::jumping(size_t t, size_t f)
{
	ASTexpr *e1 = expr1->reduce();
	ASTexpr *e2 = expr2->reduce();

	string test = e1->toString() + " " + op->toString() + " " + e2->toString();
	emitJump(test, t, f);
}

ASTor::ASTor(Token *op, ASTexpr *e1, ASTexpr *e2) : ASTlogical(op, e1, e2) { }
void ASTor::jumping(size_t t, size_t f)
{
	size_t label = t != 0 ? t : newLabel();
	expr1->jumping(label, 0);
	expr2->jumping(t, f);
	if (t == 0) emitLabel(label);
}

ASTnot::ASTnot(Token *op, ASTexpr *e1) : ASTlogical(op, e1, e1) { }
ASTexpr *ASTnot::reduce()
{
	return gen();
}
void ASTnot::jumping(size_t t, size_t f)
{
	expr1->jumping(f, t);	
}
string ASTnot::toString()
{
	return op->toString() + " " + expr1->toString();
}


ASTstmt *ASTstmt::currWhile = nullptr;
void ASTstmt::gen(size_t t, size_t f) { }

ASTbreak::ASTbreak()
{
	if (ASTstmt::currWhile == nullptr)
		error("break 语句使用错误");
	else
		stmt = ASTstmt::currWhile;
}
void ASTbreak::gen(size_t t, size_t f)
{
	emit("goto ", stmt->after);
}

ASTwhile::ASTwhile(ASTexpr *e, ASTstmt *s) : expr(e), stmt(s) { }
ASTwhile::ASTwhile() : expr(nullptr), stmt(nullptr) { }
void ASTwhile::init(ASTexpr *e, ASTstmt *s)
{
	expr = e;
	stmt = s;
}
void ASTwhile::gen(size_t b, size_t a)
{
	after = a;
	expr->jumping(0, a);
	size_t label = newLabel();
	emitLabel(label);
	if (stmt)
		stmt->gen(label, b);
	emit("goto ", b);
}

ASTassign::ASTassign(ASTid *i, ASTexpr *e) : id(i), expr(e) 
{ 
	if (id->type && expr->type)
	{
		if (!check(id->type, expr->type))
			error("类型错误");
	}
}
bool ASTassign::check(Type *tp1, Type *tp2)
{
	if (!Type::numeric(tp1) || !Type::numeric(tp2))
		return false;
	else
		return true;
}
void ASTassign::gen(size_t b, size_t a)
{
	emit(id->toString() + " = " + expr->gen()->toString());
}

ASTsetelem::ASTsetelem(ASTaccess *x, ASTexpr *e) : arr(x->arr), index(x->index), expr(e) { }
void ASTsetelem::gen(size_t b, size_t a)
{
	string s1 = index->reduce()->toString();
	string s2 = expr->reduce()->toString();
	emit(arr->toString() + "[" + s1 + "] = " + s2);
}

ASTif::ASTif(ASTexpr *e, ASTstmt *s) : expr(e), stmt(s) { }
void ASTif::gen(size_t b, size_t a)
{
	size_t label = newLabel();
	expr->jumping(0, a);
	emitLabel(label);
	if (stmt)
		stmt->gen(label, a);
}

ASTifelse::ASTifelse(ASTexpr *e, ASTstmt *s1, ASTstmt *s2) : expr(e), stmt1(s1), stmt2(s2) { }
void ASTifelse::gen(size_t b, size_t a)
{
	size_t label1 = newLabel();
	size_t label2 = newLabel();
	expr->jumping(0, label2);
	emitLabel(label1);
	if (stmt1)
		stmt1->gen(label1, a);
	emit("goto ", a);
	emitLabel(label2);
	if (stmt2)
		stmt2->gen(label2, a);
}

ASTdo::ASTdo(ASTexpr *e, ASTstmt *s) : expr(e), stmt(s) { }
ASTdo::ASTdo() : expr(nullptr), stmt(nullptr) { }
void ASTdo::init(ASTexpr *e, ASTstmt *s)
{
	expr = e;
	stmt = s;
}
void ASTdo::gen(size_t b, size_t a)
{
	after = a;
	int label = newLabel();
	if (stmt)
		stmt->gen(b, label);
	emitLabel(label);
	expr->jumping(b, 0);
}

ASTseq::ASTseq(ASTstmt *s1, ASTstmt *s2) : stmt1(s1), stmt2(s2) { }
void ASTseq::gen(size_t b, size_t a)
{
	if (stmt1 == nullptr && stmt2 == nullptr);		// 子语句都为空，什么都不做
	else if (stmt1 == nullptr) stmt2->gen(b, a);
	else if (stmt2 == nullptr) stmt1->gen(b, a);
	else
	{
		size_t label = newLabel();
		stmt1->gen(b, label);
		emitLabel(label);
		stmt2->gen(label, a);
	}
}