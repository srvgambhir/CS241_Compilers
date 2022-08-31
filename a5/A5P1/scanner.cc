#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>
#include <utility>
#include <set>
#include <array>
#include "scanner.h"

// wlp4 language scanner based on the asm scanner starter code provided in A3

// constant for max value for a token with type NUM
const int64_t MAX = 2147483647;

Token::Token(Token::Kind kind, std::string lexeme):
  kind(kind), lexeme(std::move(lexeme)) {}

  Token:: Kind Token::getKind() const { return kind; }
const std::string &Token::getLexeme() const { return lexeme; }

std::ostream &operator<<(std::ostream &out, const Token &tok) {
  switch (tok.getKind()) {
	  case Token::ID:	out << "ID";		break;
	  case Token::NUM:      out << "NUM";		break;
	  case Token::LPAREN:   out << "LPAREN";	break;
	  case Token::RPAREN:   out << "RPAREN";	break;
	  case Token::LBRACE:   out << "LBRACE";	break;
	  case Token::RBRACE:   out << "RBRACE";	break;
	  case Token::RETURN:   out << "RETURN";	break;
	  case Token::IF:       out << "IF";		break;
	  case Token::ELSE:     out << "ELSE";		break;
	  case Token::WHILE:    out << "WHILE";		break;
	  case Token::PRINTLN:  out << "PRINTLN";	break;
	  case Token::WAIN:     out << "WAIN";		break;
	  case Token::BECOMES:  out << "BECOMES";	break;
	  case Token::INT:	out << "INT";		break;
	  case Token::EQ:       out << "EQ";		break;
	  case Token::NE:       out << "NE";		break;
	  case Token::LT:       out << "LT";		break;
	  case Token::GT:       out << "GT";		break;
	  case Token::LE:       out << "LE";		break;
	  case Token::GE:       out << "GE";		break;
	  case Token::PLUS:    	out << "PLUS";		break;
	  case Token::MINUS:    out << "MINUS";		break;
	  case Token::STAR:     out << "STAR";		break;
	  case Token::SLASH:    out << "SLASH";		break;
	  case Token::PCT:      out << "PCT";		break;
	  case Token::COMMA:    out << "COMMA";		break;
	  case Token::SEMI:     out << "SEMI";		break;
	  case Token::NEW:      out << "NEW";		break;
	  case Token::DELETE:   out << "DELETE";	break;
	  case Token::LBRACK:   out << "LBRACK";	break;
	  case Token::RBRACK:   out << "RBRACK";	break;
	  case Token::AMP:      out << "AMP";		break;
	  case Token::NUL:      out << "NULL";		break;
	  case Token::WHITESPACE:	out << "WHITESPACE";	break;
	  case Token::COMMENT:	out << "COMMENT";	break;
  }
  out << " " << tok.getLexeme();
  return out;
}

int64_t Token::toNumber() const {
  std::istringstream iss;
  int64_t result;

  if (kind == NUM) {
    iss.str(lexeme);
  } else {
    // This should never happen if the user calls this function correctly
    return 0;
  }

  iss >> result;
  return result;
}

ScanningFailure::ScanningFailure(std::string message):
  message(std::move(message)) {}

const std::string &ScanningFailure::what() const { return message; }

/* Representation of a DFA, used to handle the scanning process.
 */
class AsmDFA {
  public:
    enum State {
      // States that are also kinds
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
      COMMENT,

      // States that are not also kinds
      FAIL,
      START,
      xW,
      xWA,
      xWAI,
      xWH,
      xWHI,
      xWHIL,
      xI,
      xIN,
      xE,
      xEL,
      xELS,
      xP,
      xPR,
      xPRI,
      xPRIN,
      xPRINT,
      xPRINTL,
      xR,
      xRE,
      xRET,
      xRETU,
      xRETUR,
      xCapN,
      xCapNU,
      xCapNUL,
      xN,
      xNE,
      xD,
      xDE,
      xDEL,
      xDELE,
      xDELET,
      ZERO,
      EXCLAIM,

      // Hack to let this be used easily in arrays. This should always be the
      // final element in the enum, and should always point to the previous
      // element.

      LARGEST_STATE = EXCLAIM
    };

  private:
    /* A set of all accepting states for the DFA.
     * Currently non-accepting states are not actually present anywhere
     * in memory, but a list can be found in the constructor.
     */
    std::set<State> acceptingStates;

    /*
     * The transition function for the DFA, stored as a map.
     */

    std::array<std::array<State, 128>, LARGEST_STATE + 1> transitionFunction;

    /*
     * Converts a state to a kind to allow construction of Tokens from States.
     * Throws an exception if conversion is not possible.
     */
    Token::Kind stateToKind(State s) const {
      switch(s) {
        case ID:	return Token::ID;
        case NUM:	return Token::NUM;
	case LPAREN:	return Token::LPAREN;
	case RPAREN:	return Token::RPAREN;
	case LBRACE:	return Token::LBRACE;
        case RBRACE:    return Token::RBRACE;
        case RETURN:    return Token::RETURN;
	case IF:	return Token::IF;
        case ELSE:      return Token::ELSE;
        case WHILE:	return Token::WHILE;
        case PRINTLN:	return Token::PRINTLN;
        case WAIN:    	return Token::WAIN;
	case BECOMES:	return Token::BECOMES;
	case INT:	return Token::INT;
	case EQ:	return Token::EQ;
	case NE:	return Token::NE;
	case LT:	return Token::LT;
	case GT:	return Token::GT;
	case LE:	return Token::LE;
	case GE:	return Token::GE;
	case PLUS:	return Token::PLUS;
	case MINUS:	return Token::MINUS;
	case STAR:	return Token::STAR;
	case SLASH:	return Token::SLASH;
	case PCT:	return Token::PCT;
	case COMMA:	return Token::COMMA;
	case SEMI:	return Token::SEMI;
	case NEW:	return Token::NEW;
	case DELETE:	return Token::DELETE;
	case LBRACK:	return Token::LBRACK;
	case RBRACK:	return Token::RBRACK;
	case AMP:	return Token::AMP;
	case NUL:	return Token::NUL;
	case WHITESPACE:	return Token::WHITESPACE;
	case COMMENT:	return Token::COMMENT;
	case xW:	return Token::ID;
	case xWA:	return Token::ID;
	case xWAI:	return Token::ID;
	case xWH:	return Token::ID;
	case xWHI:	return Token::ID;
	case xWHIL:	return Token::ID;
	case xI:	return Token::ID;
	case xIN:	return Token::ID;
	case xE:	return Token::ID;
	case xEL:	return Token::ID;
	case xELS:	return Token::ID;
	case xP:	return Token::ID;
	case xPR:	return Token::ID;
	case xPRI:	return Token::ID;
	case xPRIN:	return Token::ID;
	case xPRINT:	return Token::ID;
	case xPRINTL:	return Token::ID;
	case xR:	return Token::ID;
	case xRE:	return Token::ID;
	case xRET:	return Token::ID;
	case xRETU:	return Token::ID;
	case xRETUR:	return Token::ID;
	case xCapN:	return Token::ID;
	case xCapNU:	return Token::ID;
	case xCapNUL:	return Token::ID;
	case xN:	return Token::ID;
	case xNE:	return Token::ID;
	case xD:	return Token::ID;
	case xDE:	return Token::ID;
	case xDEL:	return Token::ID;
	case xDELE:	return Token::ID;
	case xDELET:	return Token::ID;
      	case ZERO:	return Token::NUM;
        default: throw ScanningFailure("ERROR: Cannot convert state to kind.");
      }
    }


  public:
    /* Tokenizes an input string according to the Simplified Maximal Munch
     * scanning algorithm.
     */
    std::vector<Token> simplifiedMaximalMunch(const std::string &input) const {
      std::vector<Token> result;

      State state = start();
      std::string munchedInput;

      // We can't use a range-based for loop effectively here
      // since the iterator doesn't always increment.
      for (std::string::const_iterator inputPosn = input.begin();
           inputPosn != input.end();) {

        State oldState = state;
        state = transition(state, *inputPosn);

        if (!failed(state)) {
          munchedInput += *inputPosn;
          oldState = state;

          ++inputPosn;
        }

        if (inputPosn == input.end() || failed(state)) {
          if (accept(oldState)) {
            result.push_back(Token(stateToKind(oldState), munchedInput));

            munchedInput = "";
            state = start();
          } else {
            if (failed(state)) {
              munchedInput += *inputPosn;
            }
            throw ScanningFailure("ERROR: Simplified maximal munch failed on input: "
                                 + munchedInput);
          }
        }
      }

      return result;
    }

    /* Initializes the accepting states for the DFA.
     */
    AsmDFA() {
      acceptingStates = {ID, NUM, LPAREN, RPAREN, LBRACE, RBRACE, RETURN, IF, ELSE, WHILE, PRINTLN,
      			 WAIN, BECOMES, INT, EQ, NE, LT, GT, LE, GE, PLUS, MINUS, STAR, SLASH,
      			 PCT, COMMA, SEMI, NEW, DELETE, LBRACK, RBRACK, AMP, NUL, WHITESPACE,
      			 COMMENT, xW, xWA, xWAI, xWH, xWHI, xWHIL, xI, xIN, xE, xEL, xELS, xP, xPR,
			 xPRI, xPRIN, xPRINT, xPRINTL, xR, xRE, xRET, xRETU, xRETUR, xCapN, xCapNU, 
			 xCapNUL, xN, xNE, xD, xDE, xDEL, xDELE, xDELET, ZERO, };
      //All other states are non-accepting

      // Initialize transitions for the DFA
      for (size_t i = 0; i < transitionFunction.size(); ++i) {
        for (size_t j = 0; j < transitionFunction[0].size(); ++j) {
          transitionFunction[i][j] = FAIL;
        }
      }

      // ID Tokens
      registerTransition(START, "abcfghjklmoqstuvxyzABCDEFGHIJKLMOPQRSTUVWXYZ", ID);
      registerTransition(ID, isalnum, ID);
      
      // NUM Tokens
      registerTransition(START, "0", ZERO);
      registerTransition(START, "123456789", NUM);
      registerTransition(NUM, isdigit, NUM);
      
      // Parentheses and Braces
      registerTransition(START, "(", LPAREN);
      registerTransition(START, ")", RPAREN);
      registerTransition(START, "{", LBRACE);
      registerTransition(START, "}", RBRACE);
      
      // RETURN Token
      registerTransition(START, "r", xR);
      registerTransition(xR, "abcdfghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xR, isdigit, ID);
      registerTransition(xR, "e", xRE);
      registerTransition(xRE, "abcdefghijklmnopqrsuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xRE, isdigit, ID);
      registerTransition(xRE, "t", xRET);
      registerTransition(xRET, "abcdefghijklmnopqrstvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xRET, isdigit, ID);
      registerTransition(xRET, "u", xRETU);
      registerTransition(xRETU, "abcdefghijklmnopqstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xRETU, isdigit, ID);
      registerTransition(xRETU, "r", xRETUR);
      registerTransition(xRETUR, "abcedfghijklmopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xRETUR, isdigit, ID);
      registerTransition(xRETUR, "n", RETURN);
      registerTransition(RETURN, isalnum, ID);
      
      // INT & IF Token
      registerTransition(START, "i", xI);
      registerTransition(xI, "abcdeghijklmopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xI, isdigit, ID);
      registerTransition(xI, "f", IF);
      registerTransition(xI, "n", xIN);
      registerTransition(xIN, "abcdefghijklmnopqrsuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xIN, isdigit, ID);
      registerTransition(xIN, "t", INT);
      registerTransition(IF, isalnum, ID);
      registerTransition(INT, isalnum, ID);

      // ELSE Token
      registerTransition(START, "e", xE);
      registerTransition(xE, "abcdefghijkmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xE, isdigit, ID);
      registerTransition(xE, "l", xEL);
      registerTransition(xEL, "abcdefghijklmnopqrtuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xEL, isdigit, ID);
      registerTransition(xEL, "s", xELS);
      registerTransition(xELS, "abcdfghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xELS, isdigit, ID);
      registerTransition(xELS, "e", ELSE);
      registerTransition(ELSE, isalnum, ID);

      // WAIN & WHILE Tokens
      registerTransition(START, "w", xW);
      registerTransition(xW, "bcdefgijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xW, isdigit, ID);
      registerTransition(xW, "a", xWA);
      registerTransition(xWA, "abcdefghjklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xWA, isdigit, ID);
      registerTransition(xWA, "i", xWAI);
      registerTransition(xWAI, "abcdefghijklmopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xWAI, isdigit, ID);
      registerTransition(xWAI, "n", WAIN);
      registerTransition(WAIN, isalnum, ID);
      registerTransition(xW, "h", xWH);
      registerTransition(xWH, "abcdefghjklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xWH, isdigit, ID);
      registerTransition(xWH, "i", xWHI);
      registerTransition(xWHI, "abcdefghijkmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xWHI, isdigit, ID);
      registerTransition(xWHI, "l", xWHIL);
      registerTransition(xWHIL, "abcdfghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xWHIL, isdigit, ID);
      registerTransition(xWHIL, "e", WHILE);
      registerTransition(WHILE, isalnum, ID);

      // PRINTLN Token
      registerTransition(START, "p", xP);
      registerTransition(xP, "abcdefghijklmnopqstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xP, isdigit, ID);
      registerTransition(xP, "r", xPR);
      registerTransition(xPR, "abcdefghjklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xPR, isdigit, ID);
      registerTransition(xPR, "i", xPRI);
      registerTransition(xPRI, "abcdefghijklmopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xPRI, isdigit, ID);
      registerTransition(xPRI, "n", xPRIN);
      registerTransition(xPRIN, "abcdefghijklmnopqrsuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xPRIN, isdigit, ID);
      registerTransition(xPRIN, "t", xPRINT);
      registerTransition(xPRINT, "abcdefghijkmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xPRINT, isdigit, ID);
      registerTransition(xPRINT, "l", xPRINTL);
      registerTransition(xPRINTL, "abcdefghijklmopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xPRINTL, isdigit, ID);
      registerTransition(xPRINTL, "n", PRINTLN);
      registerTransition(PRINTLN, isalnum, ID);

      // NEW Token
      registerTransition(START, "n", xN);
      registerTransition(xN, "abcdfghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xN, isdigit, ID);
      registerTransition(xN, "e", xNE);
      registerTransition(xNE, "abcdefghijklmnopqrstuvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xNE, isdigit, ID);
      registerTransition(xNE, "w", NEW);
      registerTransition(NEW, isalnum, ID);

      // DELETE Token
      registerTransition(START, "d", xD);
      registerTransition(xD, "abcdfghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xD, isdigit, ID);
      registerTransition(xD, "e", xDE);
      registerTransition(xDE, "abcdefghijkmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xDE, isdigit, ID);
      registerTransition(xDE, "l", xDEL);
      registerTransition(xDEL, "abcdfghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xDEL, isdigit, ID);
      registerTransition(xDEL, "e", xDELE);
      registerTransition(xDELE, "abcdefghijklmnopqrsuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xDELE, isdigit, ID);
      registerTransition(xDELE, "t", xDELET);
      registerTransition(xDELET, "abcdfghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(xDELET, isdigit, ID);
      registerTransition(xDELET, "e", DELETE);
      registerTransition(DELETE, isalnum, ID);

      // NULL Token
      registerTransition(START, "N", xCapN);
      registerTransition(xCapN, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTVWXYZ", ID);
      registerTransition(xCapN, isdigit, ID);
      registerTransition(xCapN, "U", xCapNU);
      registerTransition(xCapNU, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKMNOPQRSTUVWXYZ", ID);
      registerTransition(xCapNU, isdigit, ID);
      registerTransition(xCapNU, "L", xCapNUL);
      registerTransition(xCapNUL, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKMNOPQRSTUVWXYZ", ID);
      registerTransition(xCapNUL, isdigit, ID);
      registerTransition(xCapNUL, "L", NUL);
      registerTransition(NUL, isalnum, ID);

      // BECOMES & EQ Token
      registerTransition(START, "=", BECOMES);
      registerTransition(BECOMES, "=", EQ);
      
      // NE Token
      registerTransition(START, "!", EXCLAIM);
      registerTransition(EXCLAIM, "=", NE);

      // COMPARISON OPERATOR Tokens
      registerTransition(START, "<", LT);
      registerTransition(LT, "=", LE);
      registerTransition(START, ">", GT);
      registerTransition(GT, "=", GE);

      // PLUS & MINUS Tokens
      registerTransition(START, "+", PLUS);
      registerTransition(START, "-", MINUS);

      // More OPERATORS & COMMA/SEMI Tokens
      registerTransition(START, "*", STAR);
      registerTransition(START, "/", SLASH);
      registerTransition(SLASH, "/", COMMENT);
      registerTransition(START, "%", PCT);
      registerTransition(START, ",", COMMA);
      registerTransition(START, ";", SEMI);

      // BRACKET Tokens
      registerTransition(START, "[", LBRACK);
      registerTransition(START, "]", RBRACK);

      // AND Token
      registerTransition(START, "&", AMP);

      // WHITESPACE & COMMENT Tokens
      registerTransition(START, isspace, WHITESPACE);
      registerTransition(COMMENT, [](int c) -> int { return c != '\n'; }, COMMENT);
      registerTransition(WHITESPACE, isspace, WHITESPACE);
    }

    // Register a transition on all chars in chars
    void registerTransition(State oldState, const std::string &chars,
        State newState) {
      for (char c : chars) {
        transitionFunction[oldState][c] = newState;
      }
    }

    // Register a transition on all chars matching test
    // For some reason the cctype functions all use ints, hence the function
    // argument type.
    void registerTransition(State oldState, int (*test)(int), State newState) {

      for (int c = 0; c < 128; ++c) {
        if (test(c)) {
          transitionFunction[oldState][c] = newState;
        }
      }
    }

    /* Returns the state corresponding to following a transition
     * from the given starting state on the given character,
     * or a special fail state if the transition does not exist.
     */
    State transition(State state, char nextChar) const {
      return transitionFunction[state][nextChar];
    }

    /* Checks whether the state returned by transition
     * corresponds to failure to transition.
     */
    bool failed(State state) const { return state == FAIL; }

    /* Checks whether the state returned by transition
     * is an accepting state.
     */
    bool accept(State state) const {
      return acceptingStates.count(state) > 0;
    }

    /* Returns the starting state of the DFA
     */
    State start() const { return START; }
};

std::vector<Token> scan(const std::string &input) {
  static AsmDFA theDFA;

  std::vector<Token> tokens = theDFA.simplifiedMaximalMunch(input);

  // We need to:
  // * Throw exceptions when the value in a NUM Token exceeds the limit
  // * Remove WHITESPACE and COMMENT tokens entirely.

  std::vector<Token> newTokens;

  for (auto &token : tokens) {
	 if (token.getKind() == Token::NUM) {
		 int64_t x = token.toNumber();
		 if (x > MAX) {
			 throw ScanningFailure("ERROR: Numeric literal out of range");
		 }
	 }
	 if (token.getKind() != Token::WHITESPACE && token.getKind() != Token::Kind::COMMENT) { 
		 newTokens.push_back(token);
	 }
  }

  return newTokens;
}
