#ifndef CS241_SCANNER_H
#define CS241_SCANNER_H
#include <string>
#include <vector>
#include <set>
#include <cstdint>
#include <ostream>

// Scanner for wlp4 programming language -> based on the asm scanner starter code provided in A3

class Token;

// Scans a single line of input and produces a list of tokens.

std::vector<Token> scan(const std::string &input);

/* A scanned token produced by the scanner.
 * The "kind" tells us what kind of token it is
 * while the "lexeme" tells us exactly what text
 * the programmer typed. For example, the token
 * "abc" might have kind "ID" and lexeme "abc".
 */
class Token {
  public:
    enum Kind {
      ID = 0,
      NUM,
      LPAREN,
      RPAREN,
      LBRACE,
      RBRACE,
      RETURN,
      IF,
      ELSE,
      WHILE,
      PRINTLN,
      WAIN,
      BECOMES,
      INT,
      EQ,
      NE,
      LT,
      GT,
      LE,
      GE,
      PLUS,
      MINUS,
      STAR,
      SLASH,
      PCT,
      COMMA,
      SEMI,
      NEW,
      DELETE,
      LBRACK,
      RBRACK,
      AMP,
      NUL,
      WHITESPACE,
      COMMENT
    };

  private:
    Kind kind;
    std::string lexeme;

  public:
    Token(Kind kind, std::string lexeme);

    Kind getKind() const;
    const std::string &getLexeme() const;

    /* Converts a token to the corresponding number.
     * Only works on tokens of type NUM.
     */
    int64_t toNumber() const;
};

/* Prints a string representation of a token.
 * Mainly useful for debugging.
 */
std::ostream &operator<<(std::ostream &out, const Token &tok);

/* An exception class thrown when an error is encountered while scanning.
 */
class ScanningFailure {
    std::string message;

  public:
    ScanningFailure(std::string message);

    // Returns the message associated with the exception.
    const std::string &what() const;
};
#endif
