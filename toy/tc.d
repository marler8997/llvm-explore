// lc = Lang1 Compiler
import std.stdio;
import std.ascii : isAlpha, isAlphaNum, isDigit;
import std.array : Appender;
import std.conv : to;
import std.string : format;


private __gshared uint lineNumber;
private __gshared TextInputRange text;
int main(string[] args)
{
  precedenceMap = 
    ['<' : 10,
     '+' : 20,
     '-' : 20,
     '*' : 40];

  lineNumber = 1;

  //testLexer();
  parse();

  return 0;
}
void testLexer()
{
  text.next();

 TOKEN_LOOP:
  while(true) {
    nextToken();

    switch(currentToken) {
    case Token.eof: 
      break TOKEN_LOOP;
    case Token.def:
      writefln("def");
      break;
    case Token.extern_:
      writefln("extern_");
      break;
    case Token.id:
      writefln("id '%s'", text.currentId());
      break;
    case Token.num:
      writefln("num '%s'", text.currentId());
      break;
    default:
      writefln("char '%s'", currentToken);
      break;
    }
    stdout.flush();
  }
}



//
// Lexer
//
struct Token
{
  enum char eof     = 128;
  enum char def     = 129;
  enum char extern_ = 130;
  enum char id      = 131;
  enum char num     = 132;
}
struct TextInputRange
{
  char c;

  Appender!(char[]) saveAppender;

  void next()
  {
    int nextChar = getchar();
    if(nextChar > 127)
      throw new Exception(format("Error: character '%s' (code=%s) is invalid", cast(char)nextChar, nextChar));
    this.c = (nextChar < 0) ? Token.eof : cast(char)nextChar;
  }
  void saveToIDAndNext()
  {
    saveAppender.put(c);
    next();
  }
  void startIdAndNext()
  {
    saveAppender.clear();
    saveAppender.put(c);
    next();
  }
  char[] currentId()
  {
    return saveAppender.data;
  }
  string saveId()
  {
    return saveAppender.data.idup;
  }
  double idAsDouble()
  {
    return to!double(saveAppender.data);
  }
}
char nextTokenTemplate(T)(/*ref Token token, auto ref T text*/)
{
 NEXT_TOKEN:
  //writefln("skipping whitespace...");stdout.flush();
  while(true) {
    if(text.c != ' ' && text.c != '\t' && text.c != '\r') {
      if(text.c != '\n')
	break;
      lineNumber++;
    }
    text.next();
  }

  //writefln("checking id...");stdout.flush();
  if(isAlpha(text.c)) {
    text.startIdAndNext();
    while(isAlphaNum(text.c)) {
      text.saveToIDAndNext();
    }
    auto saved = text.currentId();
    if(saved == "def")
      return Token.def;
    if(saved == "extern")
      return Token.extern_;
    return Token.id;
  }

  //writefln("checking digit...");stdout.flush();
  if(isDigit(text.c) || text.c == '.') {
    text.startIdAndNext();
    while(isDigit(text.c) || text.c == '.') {
      text.saveToIDAndNext();
    }
    return Token.num;
  }

  //writefln("checking comment...");stdout.flush();
  if(text.c == '#') {
    while(true) {
      text.next();
      if(text.c == '\n') {
	text.next();
	goto NEXT_TOKEN;
      }
      if(text.c == Token.eof) break;
    }
  }

  //writefln("default...");stdout.flush();
  auto saveToken = text.c;
  text.next(); // read next char before next call
  return saveToken;
}

//
// AST
//
struct LlvmValue
{
}
abstract class Expr
{
  //abstract LlvmValue generateCode();
}
class NumberExpr : Expr
{
  double value;
  this(double value) {
    this.value = value;
  }
/+
  override LlvmValue generateCode()
  {
  }
+/
}
class VariableExpr : Expr
{
  string name;
  this(string name) {
    this.name = name;
  }
}
class BinaryExpr : Expr
{
  char op;
  Expr lhs, rhs;
  this(char op, Expr lhs, Expr rhs) {
    this.op = op;
    this.lhs = lhs;
    this.rhs = rhs;
  }
}
class CallExpr : Expr
{
  string name;
  Expr[] args;
  this(string name, Expr[] args) {
    this.name = name;
    this.args = args;
  }
}

class FunctionPrototype
{
  string name;
  string[] argNames;
  this(string name, string[] argNames) {
    this.name = name;
    this.argNames = argNames;
  }
}
class Function
{
  FunctionPrototype prototype; // could be null
  Expr body_;
  this(FunctionPrototype prototype, Expr body_) {
    this.prototype = prototype;
    this.body_ = body_;
  }
}

//
// Parser
//
private __gshared char currentToken;
void nextToken()
{
  currentToken = nextTokenTemplate!(TextInputRange)();
}
auto parseError(const(char)[] msg, size_t codeLine = __LINE__)
{
  //return new Exception(format("ParserError(line %d): %s", lineNumber, msg));
  writefln("ParserError(line %s): %s (parser_line %s)", lineNumber, msg, codeLine);
  stdout.flush();
  return null;
}
private __gshared ubyte[char] precedenceMap;
ubyte getPrecedence()
{
  return precedenceMap.get(currentToken, 0);
}
/// primary_expr
///   ::= id_expr
///   ::= number_expr
///   ::= paren_expr
Expr parsePrimary()
{
  switch(currentToken) {
  case Token.id: return parseIDExpr();
  case Token.num: return parseNumberExpr();
  case '(': return parseParensExpr();
  default: return parseError(format("expected expression but got token '%s'", currentToken));
  }
}
/// expression
///   ::= primary bin_op_rhs
Expr parseExpression()
{
  auto lhs = parsePrimary();
  if(!lhs) return null;
  return parseBinaryOpRHS(1, lhs);
}
/// bin_op_rhs
///   ::= ('+' primary)*
Expr parseBinaryOpRHS(ubyte exprPrecedence, Expr lhs)
{
  auto tokenPrecedence = getPrecedence();
  while(tokenPrecedence >= exprPrecedence) {

    auto binaryOp = currentToken;
    nextToken();
    
    auto rhs = parsePrimary();
    if(!rhs) return null;

    auto nextPrecedence = getPrecedence();
    if(nextPrecedence > tokenPrecedence) {
      rhs = parseBinaryOpRHS(cast(ubyte)(tokenPrecedence+1), rhs);
      if(!rhs) return null;
      nextPrecedence = getPrecedence();
    }

    lhs = new BinaryExpr(binaryOp, lhs, rhs);
    tokenPrecedence = nextPrecedence;
  }
  return lhs;
}
// id_expr ::= id
//         ::= id '(' expr* ')'
Expr parseIDExpr()
{
  auto id = text.saveId();
  nextToken();
  
  if(currentToken != '(')
    return new VariableExpr(id);

  nextToken();
  Expr[] args;
  if(currentToken != ')') {
    while(true) {
      auto arg = parseExpression();
      if(!arg) return null;
      args ~= arg;

      if(currentToken == ')')
	break;
      if(currentToken != ',')
	return parseError(format("Expected ')' or ',' in argument list but got '%s'", currentToken));
      nextToken();
    }
  }

  nextToken();

  return new CallExpr(id, args);
}
Expr parseNumberExpr()
{
  auto e = new NumberExpr(text.idAsDouble());
  nextToken();
  return e;
}
Expr parseParensExpr()
{
  nextToken();
  auto e = parseExpression();
  if(!e) return null;
  
  if(currentToken != ')')
    return parseError(format("expected ')' but got '%s'", currentToken));
  nextToken();
  return e;
}

/// func_prototype
///   ::= id '(' id* ')'
FunctionPrototype parsePrototype()
{
  if(currentToken != Token.id)
    return parseError(format("expected function name but got '%s'", currentToken));

  auto name = text.saveId();
  nextToken();

  if(currentToken != '(')
    return parseError(format("expected '(' for function arguments but got '%s'", currentToken));

  string[] argNames;
  while(true) {
    nextToken();
    if(currentToken != Token.id)
      break;
    argNames ~= text.saveId();
  }

  if(currentToken != ')')
    return parseError(format("Expected ')' at end of function arguments but got '%s'", currentToken));
  nextToken();

  return new FunctionPrototype(name, argNames);
}
Function parseFunction()
{
  nextToken(); // eat 'def'
  auto proto = parsePrototype();
  if(!proto) return null;

  auto e = parseExpression();
  if(!e) return null;
  
  return new Function(proto, e);
}
FunctionPrototype parseExtern()
{
  nextToken(); // eat 'extern'
  return parsePrototype();
}
Function parseTopLevelExpr()
{
  auto e = parseExpression();
  if(!e) return null;
  
  return new Function(null, e);
}

void handleFunction()
{
  if(parseFunction()) {
    writefln("parsed a function");
  } else {
    nextToken(); // skip token for error recovery
  }
}
void handleExtern()
{
  if(parseExtern()) {
    writefln("parsed an extern");
  } else {
    nextToken(); // skip token for error recovery
  }
}
void handleTopLevelExpr()
{
  if(parseTopLevelExpr()) {
    writefln("parsed an top-level expression");
  } else {
    nextToken(); // skip token for error recovery
  }
}
void parse()
{
  stdout.write("ready> ");
  stdout.flush();

  text.next();
  nextToken();

  while(true) {
    stdout.write("ready> ");
    stdout.flush();

    switch(currentToken) {
    case ';': nextToken(); break; // ignore top level semi-colons
    case Token.def: handleFunction(); break;
    case Token.extern_: handleExtern(); break;
    case Token.eof: return;
    default: handleTopLevelExpr(); break;
    }
  }
}
