#include <stdio.h>

#include "util.h"
#include "types.h"
#include "ast.h"

#define ASCII_LIMIT    128
#define TOKEN_EOF      128
#define TOKEN_DEF      129
#define TOKEN_EXTERN   130
#define TOKEN_ID       131
#define TOKEN_NUM      132

//
// Lexer
//
typedef struct
{
  uchar c;
  Appender saveAppender;
} TextReader;

TextReader text;
unsigned int lineNumber;

void TextReader_next(TextReader* text)
{
  int nextChar = getchar();
  if(nextChar >= ASCII_LIMIT) {
    printf("Error: character '%c' (code=%d) is not valid ascii\n", (uchar)nextChar, nextChar);
    exit(1);
  }
  text->c = (nextChar < 0) ? TOKEN_EOF : (uchar)nextChar;
}
void TextReader_startIdAndNext(TextReader* text)
{
  Appender_clear(&text->saveAppender);
  Appender_append(&text->saveAppender, text->c);
  TextReader_next(text);
}
void TextReader_saveToIDAndNext(TextReader* text)
{
  Appender_append(&text->saveAppender, text->c);
  TextReader_next(text);
}
string TextReader_currentId(TextReader* text)
{
  return string_make(text->saveAppender.ptr, text->saveAppender.length);
}

// return 0 on success
uchar TextReader_idToDouble(TextReader* text, double* output)
{
  char* buffer = text->saveAppender.ptr;
  *output = strtod(buffer, buffer + text->saveAppender.length);
  if(*output == 0.0) {
    char* p;
    for(p = buffer; p < buffer + text->saveAppender.length; p++) {
      char c = *p;
      if(c != '0' && c != '.')
	return 1; // error
    }
  }
  return 0; // success
}
string TextReader_saveId(TextReader* text)
{
  string s;
  s.length = text->saveAppender.length;
  //printf("[DEBUG] TextReader_saveId: s.length %d\n", s.length);
  s.ptr = (uchar*)malloc(s.length * sizeof(uchar));
  //printf("[DEBUG] TextReader_saveId: s.ptr %p\n", s.ptr);
  memcpy(s.ptr, text->saveAppender.ptr, s.length * sizeof(uchar));
  return s;

}

uchar lexNextToken()
{
 NEXT_TOKEN:
  //printf("skipping whitespace...\n");
  while(1) {
    if(text.c != ' ' && text.c != '\t' && text.c != '\r') {
      if(text.c != '\n')
	break;
      lineNumber++;
    }
    TextReader_next(&text);
  }

  //printf("checking id...text.c = '%c'\n", text.c);
  if(isalpha(text.c)) {
    TextReader_startIdAndNext(&text);
    while(isalnum(text.c)) {
      TextReader_saveToIDAndNext(&text);
    }
    {
      string id = TextReader_currentId(&text);
      if(id.length == 3 && strncmp(id.ptr, "def", 3) == 0)
	return TOKEN_DEF;
      if(id.length == 6 && strncmp(id.ptr, "extern", 6) == 0) {
	return TOKEN_EXTERN;
      }
    }
    return TOKEN_ID;
  }
  //printf("checking digit...\n");
  if(isdigit(text.c) || text.c == '.') {
    TextReader_startIdAndNext(&text);
    while(isdigit(text.c) || text.c == '.') {
      TextReader_saveToIDAndNext(&text);
    }
    return TOKEN_NUM;
  }
  //printf("checking comment...\n");
  if(text.c == '#') {
    do {
      TextReader_next(&text);
      if(text.c == '\n') {
	TextReader_next(&text);
	goto NEXT_TOKEN;
      }
    } while(text.c != TOKEN_EOF);
  }
  //printf("default token...\n");
  {
    uchar saveToken = text.c;
    TextReader_next(&text); // read next char before next call
    return saveToken;
  }
}
void testLexer()
{
  TextReader_next(&text);

  while(1) {
    uchar t = lexNextToken();

    switch(t) {
    case TOKEN_EOF: 
      goto FINISH_LOOP;
    case TOKEN_DEF:
      printf("KEYWORD: def\n");
      break;
    case TOKEN_EXTERN:
      printf("KEYWORD: extern\n");
      break;
    case TOKEN_ID: {
      string id = TextReader_currentId(&text);
      printf("ID     : %.*s\n", id.length, id.ptr);
      break;
    }
    case TOKEN_NUM: {
      string id = TextReader_currentId(&text);
      printf("NUMBER : %.*s\n", id.length, id.ptr);
      break;
    }
    default:
      printf("TOKEN  : '%c'\n", t);
      break;
    }
  }
 FINISH_LOOP:
  return;
}

//
// Parser
//
#define debug_parser(...)
//#define debug_parser(fmt, ...) printf("[PARSE] " fmt "\n", __VA_ARGS__);

uchar currentToken;
void nextToken()
{
  currentToken = lexNextToken();
}
#define returnParseError(fmt,...) {					\
  printf("ParseError(line %d): " fmt "\n", lineNumber, __VA_ARGS__);	\
  return 0;								\
}
uchar getPrecedence()
{
  switch(currentToken) {
  case '<': return 10;
  case '+':
  case '-': return 20;
  case '*': return 40;
  default: return 0;
  }
}
Expr parseNumberExpr()
{
  double value;
  if(TextReader_idToDouble(&text, &value)) {
    string id = TextReader_currentId(&text);
    returnParseError("invalid double literal '%.*s'", id.length, id.ptr);
  }
  {
    NumberExpr e = newNumberExpr(value);
    nextToken();
    return (Expr)e;
  }
}

Expr parseExpression();
Expr parseParensExpr()
{
  nextToken();
  {
    Expr e = parseExpression();
    if(!e) return 0;
  
    if(currentToken != ')') {
      returnParseError("expected ')' but got '%c'", currentToken);
    }
    nextToken();
    return e;
  }
}
/*
void printExprAppender(const char* prefix, ExprAppender* a)
{
  size_t i;
  printf("%s(length=%d,capcity=%d,ptr=%p) [", prefix,
	 a->length, a->capacity, a->ptr);
  for(i = 0; i < a->length; i++) {
    printf("%s%p", (i==0)?"":",", a->ptr[i]);
  }
  printf("]\n");
}
*/
// id_expr ::= id
//         ::= id '(' expr* ')'
Expr parseIDExpr()
{
  string id = TextReader_saveId(&text);
  debug_parser("--> parseIDExpr (id=\"%.*s\")", id.length, id.ptr);
  nextToken();

  if(currentToken != '(') {
    debug_parser("<-- parseIDExpr (id=\"%.*s\") VariableExpr", id.length, id.ptr);
    return (Expr)newVariableExpr(id);
  }

  nextToken();

  {
    ExprAppender args;
    ExprAppender_init(&args, 4);
    //printExprAppender("[!!!!!] ExprAppender_init()", &args);

    if(currentToken != ')') {
      while(1) {
	Expr arg = parseExpression();
	if(!arg) return 0;
	ExprAppender_append(&args, arg);

	if(currentToken == ')')
	  break;
	if(currentToken != ',')
	  returnParseError("Expected ')' or ',' in argument list but got '%c'", currentToken);
	nextToken();
      }
    }
    
    nextToken();

    return (Expr)newCallExpr(id, Expr_arr_make(args.ptr, args.length));
  }
}
/// primary_expr
///   ::= id_expr
///   ::= number_expr
///   ::= paren_expr
Expr parsePrimary()
{
  debug_parser("--> parsePrimary (currentToken='%c' %d)", currentToken, currentToken);
  switch(currentToken) {
  case TOKEN_ID : {debug_parser("  parsePrimary:id");  return parseIDExpr();}
  case TOKEN_NUM: {debug_parser("  parsePrimary:num"); return parseNumberExpr();}
  case '('      : {debug_parser("  parsePrimary:'('");   return parseParensExpr();}
  default       : returnParseError("expected expression but got token '%c'", currentToken);
  }
}
/// bin_op_rhs
///   ::= ('+' primary)*
Expr parseBinaryOpRHS(uchar exprPrecedence, Expr lhs)
{
  uchar tokenPrecedence = getPrecedence();
  debug_parser("--> parseBinaryOpRHS (currentToken='%c' %d)", currentToken, currentToken);
  while(tokenPrecedence >= exprPrecedence) {

    uchar binaryOp = currentToken;
    nextToken();
    
    {
      Expr rhs = parsePrimary();
      if(!rhs) {debug_parser("<-- parseBinaryOpRHS error (!rhs 1)");return 0;}

      {
	uchar nextPrecedence = getPrecedence();
	if(nextPrecedence > tokenPrecedence) {
	  rhs = parseBinaryOpRHS(tokenPrecedence+1, rhs);
	  if(!rhs) {debug_parser("<-- parseBinaryOpRHS error (!rhs 2)");return 0;}
	  nextPrecedence = getPrecedence();
	}

	lhs = (Expr)newBinaryExpr(binaryOp, lhs, rhs);
	tokenPrecedence = nextPrecedence;
      }
    }
  }
  debug_parser("<-- parseBinaryOpRHS");
  return lhs;
}
/// expression
///   ::= primary bin_op_rhs
Expr parseExpression()
{
  Expr lhs;

  debug_parser("--> parseExpression (currentToken = '%c' %d)", currentToken, currentToken);
  
  lhs = parsePrimary();
  if(!lhs) {
    debug_parser("<-- parseExpression error (!lhs)");
    return 0;
  }

  //return parseBinaryOpRHS(1, lhs);
  {
    Expr e = parseBinaryOpRHS(1, lhs);
    debug_parser("<-- parseExpression");
    return e;
  }
}
/// func_prototype
///   ::= id '(' id* ')'
FunctionPrototype parsePrototype()
{
  string name;

  if(currentToken != TOKEN_ID)
    returnParseError("expected function name but got '%s'", currentToken);

  name = TextReader_saveId(&text);
  nextToken();

  if(currentToken != '(')
    returnParseError("expected '(' for function arguments but got '%s'", currentToken);

  {
    StringAppender argNames;
    StringAppender_init(&argNames, 4);

    while(1) {
      nextToken();
      if(currentToken != TOKEN_ID)
	break;
      StringAppender_append(&argNames,
			    TextReader_saveId(&text));
    }

    if(currentToken != ')')
      returnParseError("Expected ')' at end of function arguments but got '%s'", currentToken);
    nextToken();

    return newFunctionPrototype(name,
				string_arr_make(argNames.ptr, argNames.length));
  }
}
Function parseFunction()
{
  FunctionPrototype proto;

  nextToken(); // eat 'def'
  proto = parsePrototype();
  if(!proto) return 0;

  {
    Expr e = parseExpression();
    if(!e) return 0;

    return newFunction(proto, e);
  }
}
FunctionPrototype parseExtern()
{
  nextToken(); // eat 'extern'
  return parsePrototype();
}
Function parseTopLevelExpr()
{
  Expr e = parseExpression();
  if(!e) return 0;
  
  return newFunction(0, e);
}
void handleFunction()
{
  if(parseFunction()) {
    printf("parsed a function\n");
  } else {
    nextToken(); // skip token for error recovery
  }
}
void handleExtern()
{
  if(parseExtern()) {
    printf("parsed an extern\n");
  } else {
    nextToken(); // skip token for error recovery
  }
}
void handleTopLevelExpr()
{  
  if(parseTopLevelExpr()) {
    printf("parsed an top-level expression\n");
  } else {
    nextToken(); // skip token for error recovery
  }
}
void parse()
{
  printf("ready> ");
  fflush(stdout);

  TextReader_next(&text);
  nextToken();

  while(1) {
    printf("ready> ");
    fflush(stdout);

    switch(currentToken) {
    case ';': nextToken(); break; // ignore top level semi-colons
    case TOKEN_DEF: handleFunction(); break;
    case TOKEN_EXTERN: handleExtern(); break;
    case TOKEN_EOF: return;
    default: handleTopLevelExpr(); break;
    }
  }
}

int main(const char* args[])
{
  Appender_init(&text.saveAppender, 256);

  lineNumber = 1;

  //testLexer();
  parse();

  return 0;
}

