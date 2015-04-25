#include "ast.h"

DefineSizedArray(Expr,Expr_arr);
DefineAppender(Expr,ExprAppender);

NumberExpr newNumberExpr(double value)
{
  NumberExpr this = (NumberExpr)malloc(sizeof(NumberExprValue));
  this->expr.type = EXPR_TYPE_NUMBER;
  this->value     = value;
  return this;
}
VariableExpr newVariableExpr(string name)
{
  VariableExpr this = (VariableExpr)malloc(sizeof(VariableExprValue));
  this->expr.type = EXPR_TYPE_VARIABLE;
  this->name      = name;
  return this;
}
BinaryExpr newBinaryExpr(uchar op, Expr lhs, Expr rhs)
{
  BinaryExpr this = (BinaryExpr)malloc(sizeof(BinaryExprValue));
  this->expr.type = EXPR_TYPE_BINARY;
  this->op        = op;
  this->lhs       = lhs;
  this->rhs       = rhs;
  return this;
}
CallExpr newCallExpr(string name, Expr_arr args)
{
  CallExpr this = (CallExpr)malloc(sizeof(CallExprValue));
  this->expr.type = EXPR_TYPE_CALL;
  this->name      = name;
  this->args      = args;
  return this;
}
FunctionPrototype newFunctionPrototype(string name, string_arr argNames)
{
  FunctionPrototype this = (FunctionPrototype)malloc(sizeof(FunctionPrototypeValue));
  this->name = name;
  this->argNames = argNames;
  return this;
}
Function newFunction(FunctionPrototype prototype, Expr body)
{
  Function this = (Function)malloc(sizeof(FunctionValue));
  this->prototype = prototype;
  this->body = body;
  return this;
}

//
// Print Visitor
//
void printNumberExpr(struct AstVisitor* visitor, NumberExpr e)
{
  printf("NumberExpr '%lf'\n", e->value);
}
void printVariableExpr(struct AstVisitor* visitor, VariableExpr e)
{
  printf("VariableExpr '%.*s'\n", e->name.length, e->name.ptr);
}
void printBinaryExpr(struct AstVisitor* visitor, BinaryExpr e)
{
  printf("BinaryExpr '%c'\n", e->op);
  //printf("<-- BinaryExpr '%c'\n", e->op);
  //e->lhs->expr->accept(visitor, e->lhs);
  //e->rhs->expr->accept(visitor, e->rhs);
}
void printCallExpr(struct AstVisitor* visitor, CallExpr e)
{
}
void initializePrintVisitor(AstVisitor* visitor)
{
  visitor->visitNumberExpr = printNumberExpr;
  visitor->visitVariableExpr = printVariableExpr;
  visitor->visitBinaryExpr = printBinaryExpr;
  visitor->visitCallExpr = printCallExpr;
}
