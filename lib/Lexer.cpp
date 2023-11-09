#include <lluac/Lexer.h>

#include <llvm/ADT/StringExtras.h>
#include <llvm/Support/FormatVariadic.h>

#include <cstdlib>

using namespace lluac;

#define LEXER_PRINT_ERROR_AND_EXIT(sourceMgr, errorStr, offendingVal)          \
  do {                                                                         \
    llvm::SMLoc startLoc = llvm::SMLoc::getFromPointer(offendingVal.data());   \
    llvm::SMLoc endLoc = llvm::SMLoc::getFromPointer(offendingVal.data() +     \
                                                     offendingVal.size());     \
    std::string errorMsg =                                                     \
        llvm::formatv("{0}: '{1}'", errorStr, offendingVal);                   \
    sourceMgr.PrintMessage(llvm::errs(), startLoc, llvm::SourceMgr::DK_Error,  \
                           errorMsg, {{startLoc, endLoc}});                    \
    std::exit(1);                                                              \
  } while (false);                                                             \
  llvm_unreachable("Control flow should never reach this point in the code");

static bool IsValidDouble(llvm::StringRef number) {
  double x;
  return !number.getAsDouble(x, /*AllowInexact=*/true);
}

Lexer::Result Lexer::lex() {
begin:
  Buffer = Buffer.drop_while(llvm::isSpace);
  if (Buffer.empty()) {
    return {Token::EndOfFile, Buffer};
  }

  if (Buffer.starts_with("--[[")) {
    size_t endTok = Buffer.find("]]");
    if (endTok == llvm::StringRef::npos) {
      llvm::StringRef startTok = Buffer.take_front(4);
      LEXER_PRINT_ERROR_AND_EXIT(SourceMgr, "unterminated comment", startTok);
    }
    Buffer = Buffer.drop_front(endTok + 2);
    goto begin;
  }
  if (Buffer.starts_with("--")) {
    Buffer = Buffer.drop_until([](char c) { return c == '\n' || c == '\r'; });
    goto begin;
  }

  char c = Buffer.front();
  switch (c) {
  case '=':
    Buffer = Buffer.drop_front();
    return {Token::Equals, "="};
  case '(':
    Buffer = Buffer.drop_front();
    return {Token::LeftParen, "("};
  case ')':
    Buffer = Buffer.drop_front();
    return {Token::RightParen, ")"};
  case '[':
    Buffer = Buffer.drop_front();
    return {Token::LeftSquare, "["};
  case ']':
    Buffer = Buffer.drop_front();
    return {Token::RightSquare, "]"};
  case '{':
    Buffer = Buffer.drop_front();
    return {Token::LeftBracket, "{"};
  case '}':
    Buffer = Buffer.drop_front();
    return {Token::RightBracket, "}"};
  case ',':
    Buffer = Buffer.drop_front();
    return {Token::Comma, ","};
  default:
    break;
  }

  if (llvm::isAlpha(c) || c == '_') {
    llvm::StringRef identifier =
        Buffer.take_while([](char c) { return llvm::isAlnum(c) || c == '_'; });
    Buffer = Buffer.drop_front(identifier.size());
    Token tok = llvm::StringSwitch<Token>{identifier}
                    .Case("local", Token::Local)
                    .Case("function", Token::Function)
                    .Case("return", Token::Return)
                    .Case("if", Token::If)
                    .Case("else", Token::Else)
                    .Case("elseif", Token::ElseIf)
                    .Case("then", Token::Then)
                    .Case("for", Token::For)
                    .Case("do", Token::Do)
                    .Case("and", Token::And)
                    .Case("break", Token::Break)
                    .Case("in", Token::In)
                    .Case("not", Token::Not)
                    .Case("or", Token::Or)
                    .Case("repeat", Token::Repeat)
                    .Case("until", Token::Until)
                    .Case("while", Token::While)
                    .Case("end", Token::End)
                    .Case("true", Token::True)
                    .Case("false", Token::False)
                    .Case("nil", Token::Nil)
                    .Default(Token::Identifier);
    return {tok, identifier};
  }
  if (llvm::isDigit(c) || c == '.') {
    llvm::StringRef number = Buffer.take_while([](char c) {
      return llvm::isDigit(c) || c == '.' || c == 'e' || c == '-' || c == '+';
    });
    Buffer = Buffer.drop_front(number.size());
    if (!IsValidDouble(number)) {
      LEXER_PRINT_ERROR_AND_EXIT(SourceMgr, "invalid number", number);
    }
    return {Token::Number, number};
  }

  llvm::StringRef unknownTok = Buffer.take_until(llvm::isSpace);
  Buffer = Buffer.drop_front(unknownTok.size());
  LEXER_PRINT_ERROR_AND_EXIT(SourceMgr, "unknown token", unknownTok);
}

#undef LEXER_PRINT_ERROR_AND_EXIT
