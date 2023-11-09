#ifndef LLUAC_LEXER_H
#define LLUAC_LEXER_H

#include <llvm/Support/SourceMgr.h>

namespace lluac {

class Lexer {
public:
  enum class Token {
    EndOfFile,
    Identifier,
    Local,
    Function,
    Return,
    If,
    Else,
    ElseIf,
    Then,
    For,
    Do,
    And,
    Break,
    In,
    Not,
    Or,
    Repeat,
    Until,
    While,
    End,
    True,
    False,
    Nil,
    Number,
    Equals,
    LeftParen,
    RightParen,
    LeftSquare,
    RightSquare,
    LeftBracket,
    RightBracket,
    Comma,
  };

  struct Result {
    Token Token;
    llvm::StringRef Value;
  };

  Result lex();

  Lexer(const llvm::SourceMgr &sourceMgr) : SourceMgr(sourceMgr) {
    assert(sourceMgr.getNumBuffers() && "No source file provided");
    Buffer = sourceMgr.getMemoryBuffer(sourceMgr.getMainFileID())->getBuffer();
  }

private:
  llvm::StringRef Buffer;
  const llvm::SourceMgr &SourceMgr;
};

} // namespace lluac

#endif // LLUAC_LEXER_H
