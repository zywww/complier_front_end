#ifndef AST_H
#define AST_H

#include "Token.h"
#include "Lexer.h"

// AST 树的每个节点都属于 ASTnode 类
class ASTnode
{
protected:
	static size_t label;			// 用于中间代码的标号
	size_t	lexLine = 0;			// 每个结点构造对应在代码文件的行号

	
public:
	static bool wrong_happen;

	ASTnode();

	size_t newLabel();								// 返回一个新的标号
	void error(std::string info);					// 输出错误信息
	void emitLabel(size_t lb);						// 生成中间代码，输出标号
	void emit(std::string str, size_t lb = -1);		// 生成中间代码，输出语句
};

// 表达式的根类，每个表达式至少有一个符号和结果类型，对于 ASTid 等结点，符号是 id 词法单元
class ASTexpr : public ASTnode
{
public:
	Token *op;		// 表达式运算符
	Type *type;		// 表达式结果类型
public:
	ASTexpr(Token *o, Type *tp);

	virtual ASTexpr* gen();				// 返回一个项，该项可以成为一个三地址指令的右部
	virtual ASTexpr* reduce();			// 把一个表达式计算成为一个单一的地址

	// t 为表达式为 true 时的出口，f 则为 false 时的出口，标号为 0 时，表示直接继续下一语句
	virtual	void jumping(size_t t, size_t f);		// 逻辑表达式会重新实现该函数
	virtual void emitJump(std::string test, size_t t, size_t f);
	virtual std::string toString();
};

// 常量类，整型常量，浮点型常量，布尔常量
class ASTconstant : public ASTexpr
{
public:
	static ASTconstant *booltrue;
	static ASTconstant *boolfalse;

	ASTconstant(Token *op, Type *tp);

	virtual void jumping(size_t t, size_t f);
};

// 中间变量类，在规约表达式时，所需要的中间变量，也属于 AST 结点
class ASTtemp : public ASTexpr
{
protected:
	static size_t count;		// 在构造中间变量时，将值作为中间变量的序号数，并自增
	size_t number = 0;			// 中间变量的序号数
public:
	ASTtemp(Type *tp);

	virtual std::string toString();
};

// 标识符类
class ASTid : public ASTexpr
{
protected:
	size_t offset;	// 相对地址，一个标识符就是一个地址
public:
	ASTid(Token *id, Type *tp, size_t lc);
};

// 一元运算符、二元的算术运算符和数组元素类将继承本类，他们有共同的 reduce 函数
class ASTop : public ASTexpr
{
public:
	ASTop(Token *op, Type *tp);

	virtual ASTexpr* reduce();	// 其子类都不覆盖给函数
};

// 数组元素类，相对于 ASTid 类，它多出了下标元素
class ASTaccess : public ASTop
{
public:
	ASTid *arr;
	ASTexpr *index;
public:
	ASTaccess(ASTid *a, ASTexpr *e, Type *p);	// p 是将数组平坦后的类型，即基本类型

	virtual ASTexpr* gen();
	virtual std::string toString();
};

// 二元算术类型，包括 + - * / 
class ASTarith : public ASTop
{
protected:
	ASTexpr *expr1;
	ASTexpr *expr2;
public:
	ASTarith(Token *op, ASTexpr *e1, ASTexpr *e2);

	virtual ASTexpr* gen();
	virtual std::string toString();
};

// 一元运算符，在本程序只用于 - （负号）
class ASTunary : public ASTop
{
protected:
	ASTexpr *expr;
public:
	ASTunary(Token *op, ASTexpr *e);

	virtual ASTexpr* gen();
	virtual std::string toString();
};

// 逻辑运算符类，逻辑运算符不计算本身结果，只使用因子结果，所以不继承 ASTop 类，因为有不同的实现
// 且其有短路的特点，所以生成的中间代码不同，需要跳转语句
class ASTlogical : public ASTexpr
{
protected:
	ASTexpr *expr1;
	ASTexpr *expr2;
public:
	ASTlogical(Token *op, ASTexpr *e1, ASTexpr *e2);

	virtual Type* check(Type *tp1, Type *tp2);	// 在关系运算符类中会覆盖
	virtual ASTexpr* gen();					// 只有在将逻辑表达式赋值给变量，才会用到此函数
	virtual std::string toString();
};

// 与运算符类
class ASTand : public ASTlogical
{
public:
	ASTand(Token *op, ASTexpr *e1, ASTexpr *e2);
	
	//virtual ASTexpr* reduce() { return gen(); }
	virtual void jumping(size_t t, size_t f);
};

// 关系运算符类 包括< <= > >= == !=
class ASTrel : public ASTlogical
{
public:
	ASTrel(Token *op, ASTexpr *e1, ASTexpr *e2);
	
	virtual ASTexpr* reduce() { return gen(); }
	virtual void jumping(size_t t, size_t f);
};

// 或运算符类
class ASTor : public ASTlogical
{
public:
	ASTor(Token *op, ASTexpr *e1, ASTexpr *e2);

	virtual void jumping(size_t t, size_t f);
};

// 非运算符类
class ASTnot : public ASTlogical
{
public:
	ASTnot(Token *op, ASTexpr *e1);

	virtual ASTexpr* reduce();
	virtual void jumping(size_t t, size_t f);
	virtual std::string toString();
};

// 语句基类，其他语句类都继承与此类
class ASTstmt : public ASTnode
{
public:
	size_t after = 0;			// break 语句可以使用该标号跳转
	static ASTstmt *currWhile;
	
	ASTstmt() { }

	virtual void gen(size_t t, size_t f);
};

// break 语句类，其中的成员 stmt 是记录 break 语句所在的循环语句
class ASTbreak : public ASTstmt
{
protected:
	ASTstmt *stmt;
public:
	ASTbreak();

	virtual void gen(size_t t, size_t f);
};


class ASTwhile : public ASTstmt
{
protected:
	ASTexpr *expr;
	ASTstmt *stmt;
public:
	ASTwhile(ASTexpr *e, ASTstmt *s);
	ASTwhile();

	void init(ASTexpr *e, ASTstmt *s);
	virtual void gen(size_t t, size_t f);
};

// 赋值语句类
class ASTassign : public ASTstmt
{
protected:
	ASTid *id;
	ASTexpr *expr;
public:
	ASTassign(ASTid *i, ASTexpr *e);

	bool check(Type *tp1, Type *tp2);
	virtual void gen(size_t b, size_t a);
};

// 赋值语句类，但是将结果赋给数组元素，所以多了个下标成员
class ASTsetelem : public ASTstmt
{
protected:
	ASTid *arr;
	ASTexpr *index;
	ASTexpr *expr;
public:
	ASTsetelem(ASTaccess *x, ASTexpr *e);

	virtual void gen(size_t b, size_t a);
};

// 没有 else 的 if 语句
class ASTif : public ASTstmt
{
protected:
	ASTexpr *expr;
	ASTstmt *stmt;
public:
	ASTif(ASTexpr *e, ASTstmt *s);


	virtual void gen(size_t b, size_t a);
};

// if else 语句
class ASTifelse : public ASTstmt
{
protected:
	ASTexpr *expr;
	ASTstmt *stmt1;
	ASTstmt *stmt2;
public:
	ASTifelse(ASTexpr *e, ASTstmt *s1, ASTstmt *s2);

	virtual void gen(size_t b, size_t a);
};

// do while 语句
class ASTdo : public ASTstmt
{
protected:
	ASTexpr *expr;
	ASTstmt *stmt;
public:
	ASTdo(ASTexpr *e, ASTstmt *s);
	ASTdo();

	void init(ASTexpr *e, ASTstmt *s);
	virtual void gen(size_t b, size_t a);
};

// 语句序列，有两个成员，构成了一棵语句树，向右下角伸展
class ASTseq : public ASTstmt
{
protected:
	ASTstmt *stmt1;
	ASTstmt *stmt2;
public:
	ASTseq(ASTstmt *s1, ASTstmt *s2);

	virtual void gen(size_t b, size_t a);
};
#endif