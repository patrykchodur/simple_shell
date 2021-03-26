#include "lexer.hpp"
#include <iostream>

// std::isdigit etc
#include <cctype>

Lexer::Lexer(bool interactive) {
	m_interactive = interactive;
	m_was_escaped = false;
	m_was_redirection = false;
}

int Lexer::get_char() {
	return m_stream->get();
}

void Lexer::unget_char(int c) {
	m_stream->unget();
}

bool Lexer::is_identifier_char(int c) {
	if (std::isalnum(c))
		return true;

	switch (c) {
	case '-':
	case '_':
	case '.':
	case '/':
	case '~':
	case '=':
	case '?':
		return true;
	}

	return false;
}

#define LEX_RETURN(a, b) do { \
		m_was_escaped = false; \
		m_was_redirection = false; \
		return { (a), (b) }; \
	} while (0)

std::pair<Token, std::string> Lexer::lex() {
	auto character = get_char();

	auto eof = -1;
	if (character == eof)
		LEX_RETURN(_EOF, "");

	// some workarounds around bash stupid rules
	if (m_was_redirection && character == '&') {
		std::string result;
		while (std::isdigit(character = get_char()))
			result += character;
		if (is_identifier_char(character)) {
			do {
				result += character;
			} while (is_identifier_char(character = get_char()));
			unget_char(character);
			LEX_RETURN(_ARGUMENT, result);
		}
		unget_char(character);
		LEX_RETURN(_FILDES, result);
	}

	if (std::isdigit(character)) {
		std::string result;

		do {
			result += character;
		} while (std::isdigit(character = get_char()));

		if (is_identifier_char(character)) {
			do {
				result += character;
			} while (is_identifier_char(character = get_char()));
			unget_char(character);
			LEX_RETURN( _ARGUMENT, result);
		}

		if (character == '>') {
			unget_char(character);
			LEX_RETURN( _FILDES, result);
		}
		unget_char(character);
		LEX_RETURN(_ARGUMENT, result);
	}

	if (is_identifier_char(character)) {
		std::string result;
		do {
			result += character;
		}
		while (is_identifier_char(character = get_char()));
		unget_char(character);
		LEX_RETURN(_ARGUMENT, result);
	}

	switch (character) {
	case '&':
	{
		auto tmp = get_char();
		if (tmp == '&')
			LEX_RETURN(_ANDAND, "&&");
		unget_char(tmp);
		LEX_RETURN(_AND, "&");
	}

	case '|':
	{
		auto tmp = get_char();
		if (tmp == '|')
			LEX_RETURN(_OROR, "||");
		unget_char(tmp);
		LEX_RETURN(_PIPE, "|");
	}

	case '\n':
		if (m_was_escaped)
			return lex();
		// fallthrough
		if (m_interactive)
			LEX_RETURN( _EOF, "\n");
	case ';':
		LEX_RETURN(_SEPARATOR, std::string(1, character));

	case '\\':
		m_was_escaped = true;
		m_was_redirection = false;
		return lex();

	case '$':
	{
		std::string result;
		while (is_identifier_char(character = get_char()))
			result += character;
		unget_char(character);
		LEX_RETURN(_VARIABLE, result);
	}

	case '>':
	{
		auto tmp = get_char();
		if (tmp == '>')
			LEX_RETURN(_APPEND_REDIRECTION, ">>");
		unget_char(tmp);
		m_was_redirection = true;
		m_was_escaped = false;
		return { _OUT_REDIRECTION, ">" };
	}
	
	case '<':
		m_was_redirection = true;
		m_was_escaped = false;
		return { _IN_REDIRECTION, "<" };
	case '"':
	case '\'':
	{
		auto tmp = character;
		std::string result;
		// result += tmp;
		while ((character = get_char()) != tmp)
			result += character;
		LEX_RETURN(_ARGUMENT, result);
	}

	case ' ':
	case '\t':
		return lex();

	case '#':
	{
		auto tmp = get_char();
		while (tmp != '\n' && tmp != eof)
			tmp = get_char();
		unget_char(tmp);
		return lex();
	}

	}

	LEX_RETURN(_ERROR, std::string(1, character));
}

void Lexer::set_stream(std::istream* stream) {
	m_stream = stream;
}
