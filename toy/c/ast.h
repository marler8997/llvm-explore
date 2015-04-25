#ifndef AST_H
#define AST_H

#include "util.h"
#include "types.h"

typedef struct AstVisitor;

typedef enum {
  EXPR_TYPE_NUMBER   = 0,
  EXPR_TYPE_VARIABLE = 1,
  EXPR_TYPE_BINARY   = 2,
  EXPR_TYPE_CALL     = 3,
} ExprType;

typedef struct {
  ExprType type;
} *Expr, ExprValue;
DeclareSizedArray(Expr,Expr_arr);
DeclareAppender(Expr,ExprAppender);

//void acceptNumberExpr(struct AstVisitor
typedef struct {
  ExprValue expr; // base class

  double value;
} *NumberExpr, NumberExprValue;
NumberExpr newNumberExpr(double value);

typedef struct {
  ExprValue expr; // base class

  string name;
} *VariableExpr, VariableExprValue;
VariableExpr newVariableExpr(string name);

typedef struct {
  ExprValue expr; // base class

  uchar op;
  Expr lhs, rhs;
} *BinaryExpr, BinaryExprValue;
BinaryExpr newBinaryExpr(uchar op, Expr lhs, Expr rhs);

typedef struct {
  ExprValue expr; // base class

  string name;
  Expr_arr args;
} *CallExpr, CallExprValue;
CallExpr newCallExpr(string name, Expr_arr args);

typedef struct
{
  string name;
  string_arr argNames;
} *FunctionPrototype, FunctionPrototypeValue;
FunctionPrototype newFunctionPrototype(string name, string_arr argNames);

typedef struct
{
  FunctionPrototype prototype; // could be null
  Expr body;
} *Function, FunctionValue;
Function newFunction(FunctionPrototype prototype, Expr body);

//
// Visitor
//
typedef struct {
  void (*visitNumberExpr  )(struct AstVisitor* visitor, NumberExpr e);
  void (*visitVariableExpr)(struct AstVisitor* visitor, VariableExpr e);
  void (*visitBinaryExpr  )(struct AstVisitor* visitor, BinaryExpr e);
  void (*visitCallExpr    )(struct AstVisitor* visitor, CallExpr e);
} AstVisitor;

struct ExprVtable {
  void (*accept)(struct AstVisitor* visitor, Expr expr);
}



#endif
