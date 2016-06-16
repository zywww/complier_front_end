#ifndef AST_H
#define AST_H

#include "Token.h"
#include "Lexer.h"

// AST ����ÿ���ڵ㶼���� ASTnode ��
class ASTnode
{
protected:
	static size_t label;			// �����м����ı��
	size_t	lexLine = 0;			// ÿ����㹹���Ӧ�ڴ����ļ����к�

	
public:
	static bool wrong_happen;

	ASTnode();

	size_t newLabel();								// ����һ���µı��
	void error(std::string info);					// ���������Ϣ
	void emitLabel(size_t lb);						// �����м���룬������
	void emit(std::string str, size_t lb = -1);		// �����м���룬������
};

// ���ʽ�ĸ��࣬ÿ�����ʽ������һ�����źͽ�����ͣ����� ASTid �Ƚ�㣬������ id �ʷ���Ԫ
class ASTexpr : public ASTnode
{
public:
	Token *op;		// ���ʽ�����
	Type *type;		// ���ʽ�������
public:
	ASTexpr(Token *o, Type *tp);

	virtual ASTexpr* gen();				// ����һ���������Գ�Ϊһ������ַָ����Ҳ�
	virtual ASTexpr* reduce();			// ��һ�����ʽ�����Ϊһ����һ�ĵ�ַ

	// t Ϊ���ʽΪ true ʱ�ĳ��ڣ�f ��Ϊ false ʱ�ĳ��ڣ����Ϊ 0 ʱ����ʾֱ�Ӽ�����һ���
	virtual	void jumping(size_t t, size_t f);		// �߼����ʽ������ʵ�ָú���
	virtual void emitJump(std::string test, size_t t, size_t f);
	virtual std::string toString();
};

// �����࣬���ͳ����������ͳ�������������
class ASTconstant : public ASTexpr
{
public:
	static ASTconstant *booltrue;
	static ASTconstant *boolfalse;

	ASTconstant(Token *op, Type *tp);

	virtual void jumping(size_t t, size_t f);
};

// �м�����࣬�ڹ�Լ���ʽʱ������Ҫ���м������Ҳ���� AST ���
class ASTtemp : public ASTexpr
{
protected:
	static size_t count;		// �ڹ����м����ʱ����ֵ��Ϊ�м�������������������
	size_t number = 0;			// �м�����������
public:
	ASTtemp(Type *tp);

	virtual std::string toString();
};

// ��ʶ����
class ASTid : public ASTexpr
{
protected:
	size_t offset;	// ��Ե�ַ��һ����ʶ������һ����ַ
public:
	ASTid(Token *id, Type *tp, size_t lc);
};

// һԪ���������Ԫ�����������������Ԫ���ཫ�̳б��࣬�����й�ͬ�� reduce ����
class ASTop : public ASTexpr
{
public:
	ASTop(Token *op, Type *tp);

	virtual ASTexpr* reduce();	// �����඼�����Ǹ�����
};

// ����Ԫ���࣬����� ASTid �࣬��������±�Ԫ��
class ASTaccess : public ASTop
{
public:
	ASTid *arr;
	ASTexpr *index;
public:
	ASTaccess(ASTid *a, ASTexpr *e, Type *p);	// p �ǽ�����ƽ̹������ͣ�����������

	virtual ASTexpr* gen();
	virtual std::string toString();
};

// ��Ԫ�������ͣ����� + - * / 
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

// һԪ��������ڱ�����ֻ���� - �����ţ�
class ASTunary : public ASTop
{
protected:
	ASTexpr *expr;
public:
	ASTunary(Token *op, ASTexpr *e);

	virtual ASTexpr* gen();
	virtual std::string toString();
};

// �߼�������࣬�߼�����������㱾������ֻʹ�����ӽ�������Բ��̳� ASTop �࣬��Ϊ�в�ͬ��ʵ��
// �����ж�·���ص㣬�������ɵ��м���벻ͬ����Ҫ��ת���
class ASTlogical : public ASTexpr
{
protected:
	ASTexpr *expr1;
	ASTexpr *expr2;
public:
	ASTlogical(Token *op, ASTexpr *e1, ASTexpr *e2);

	virtual Type* check(Type *tp1, Type *tp2);	// �ڹ�ϵ��������лḲ��
	virtual ASTexpr* gen();					// ֻ���ڽ��߼����ʽ��ֵ���������Ż��õ��˺���
	virtual std::string toString();
};

// ���������
class ASTand : public ASTlogical
{
public:
	ASTand(Token *op, ASTexpr *e1, ASTexpr *e2);
	
	//virtual ASTexpr* reduce() { return gen(); }
	virtual void jumping(size_t t, size_t f);
};

// ��ϵ������� ����< <= > >= == !=
class ASTrel : public ASTlogical
{
public:
	ASTrel(Token *op, ASTexpr *e1, ASTexpr *e2);
	
	virtual ASTexpr* reduce() { return gen(); }
	virtual void jumping(size_t t, size_t f);
};

// ���������
class ASTor : public ASTlogical
{
public:
	ASTor(Token *op, ASTexpr *e1, ASTexpr *e2);

	virtual void jumping(size_t t, size_t f);
};

// ���������
class ASTnot : public ASTlogical
{
public:
	ASTnot(Token *op, ASTexpr *e1);

	virtual ASTexpr* reduce();
	virtual void jumping(size_t t, size_t f);
	virtual std::string toString();
};

// �����࣬��������඼�̳������
class ASTstmt : public ASTnode
{
public:
	size_t after = 0;			// break ������ʹ�øñ����ת
	static ASTstmt *currWhile;
	
	ASTstmt() { }

	virtual void gen(size_t t, size_t f);
};

// break ����࣬���еĳ�Ա stmt �Ǽ�¼ break ������ڵ�ѭ�����
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

// ��ֵ�����
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

// ��ֵ����࣬���ǽ������������Ԫ�أ����Զ��˸��±��Ա
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

// û�� else �� if ���
class ASTif : public ASTstmt
{
protected:
	ASTexpr *expr;
	ASTstmt *stmt;
public:
	ASTif(ASTexpr *e, ASTstmt *s);


	virtual void gen(size_t b, size_t a);
};

// if else ���
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

// do while ���
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

// ������У���������Ա��������һ��������������½���չ
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