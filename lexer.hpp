#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <utility>

enum Token {
	_ERROR = -1,
	_EOF = 0,
	_ARGUMENT, // ./cmd, -t, test, 'test 2'
	_PIPE, // |
	_AND, // &
	_ANDAND, // &&
	_OROR, // ||
	_SEPARATOR, // ;, <NewLine>
	_OUT_REDIRECTION, // > 
	_APPEND_REDIRECTION, // >>
	_IN_REDIRECTION, // <
	_FILDES, // 4, &2 (recognized close to redirection)

	_TOKEN_LAST,
};

class Lexer {
public:
	Lexer(bool interactive);
	std::pair<Token, std::string> lex();
	operator bool();
private:
	int get_char();
	void unget_char(int character);
	bool is_identifier_char(int c);
	bool m_was_escaped;
	bool m_was_redirection;
	bool m_interactive;
};

#endif
