#ifndef PARSER_HPP
#define PARSER_HPP

#include "lexer.hpp"
#include "program.hpp"
#include <vector>

/*
 * GRAMMAR RULES
 *
 * root = [ pipeline, { pipeline_separator, pipeline } ] _EOF;
 *
 * pipeline_separator =  _SEPARATOR
 *                     | _AND
 *                     | _ANDAND
 *                     | _OROR
 *                     ;
 *
 * pipeline = command { _PIPE command };
 *
 * command = { _ARGUMENT | redirection };
 *
 * redirection = [ _FILDES ], redirection_operator, redirection_argument;
 *
 * redirection_argument = _FILDES | _ARGUMENT;
 *
 * redirection_operator =  _OUT_REDIRECTION
 *                       | _APPEND_REDIRECTION
 *                       | _IN_REDIRECTION
 *                       ;
 *
 * 
 */

class Parser {
public:
	Parser(bool interactive);
	void main_loop();

private:
	Lexer m_lex;
	bool m_interactive;
	bool m_finish;
	std::vector<std::pair<Token, std::string>> m_token_stack;
	size_t m_token_stack_position;
	int m_last_return_value;
	std::vector<Program> m_programs_to_run;

	bool m_error;

	bool accept(Token token);
	bool expect(Token token);
	bool ask(Token token);
	bool ask(const std::initializer_list<Token>& token_list);
	void next_token();
	void previous_token();
	void unexpected_token(Token token);
	void token_error();
	void error(const std::string& message);
	Token current_token();
	const std::string& current_text();

	void root();
	void pipeline();
	void pipeline_separator();
	void command();
	void redirection();

	int last_return_value();
	void execute_to_run();
	void print_prompt();
};

#endif
