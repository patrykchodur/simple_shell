#include "parser.hpp"
#include "low_level.h"
#include <iostream>

Parser::Parser(bool interactive) :
		m_lex(interactive),
		m_interactive(interactive) {
	m_token_stack_position = -1;
	m_finish = false;
	m_last_return_value = 0;
	m_error = false;

}

bool Parser::accept(Token token) {
	if (current_token() == token) {
		next_token();
		return true;
	}
	return false;
}

bool Parser::ask(Token token) {
	return current_token() == token;
}

bool Parser::ask(const std::initializer_list<Token>& token_list) {
	for (auto&& iter : token_list)
		if (ask(iter))
			return true;
	return false;
}

bool Parser::expect(Token token) {
	if (accept(token))
		return true;

	unexpected_token(current_token());
	return false;
}

void Parser::next_token() {
	if (m_token_stack_position < m_token_stack.size() - 1)
		;
	else
		m_token_stack.push_back(m_lex.lex());
	++m_token_stack_position;

	if (current_token() < 0)
		token_error();
}

void Parser::unexpected_token(Token token) {
	assert(0);
}

void Parser::token_error() {
	m_error = true;
	switch (current_token()) {
	case _ERROR:
		error("Unknown token");
		break;

	default:
		error("Unknown error in file");
	}
}

void Parser::error(const std::string& message) {
	std::cerr << message << '\n';

}

void Parser::previous_token() {
	--m_token_stack_position;
}

Token Parser::current_token() {
	return m_token_stack[m_token_stack_position].first;
}

const std::string& Parser::current_text() {
	return m_token_stack[m_token_stack_position - 1].second;
}

void Parser::root() {
	next_token();
	while (!m_error) {
		if (ask(_EOF))
			break;
		pipeline();
		if (accept(_SEPARATOR)) {
			execute_to_run();
			continue;
		}
		if (accept(_AND)) {
			continue;
		}
		if (accept(_ANDAND)) {
			execute_to_run();
			if (last_return_value() != 0)
				break;
		}
		if (accept(_OROR)) {
			execute_to_run();
			if (last_return_value() == 0)
				break;
		}
	}
	execute_to_run();
}

void Parser::pipeline() {
	command();
}

void Parser::pipeline_separator() {
	assert(0);
}

void Parser::command() {
	Program result;
	while (ask({_VARIABLE, _ARGUMENT, _OUT_REDIRECTION, _APPEND_REDIRECTION, _IN_REDIRECTION, _FILDES})) {
		if (accept(_ARGUMENT)) {
			if (result.m_name.empty())
				result.m_name = current_text();
			else {
				result.m_args.emplace_back(current_text());
			}
			continue;
		}
		else if (accept(_VARIABLE)) {
			if (result.m_name.empty()) {
				auto tmp = get_env(current_text().c_str());
				if (tmp)
					result.m_name = tmp;
			} else {
				auto tmp = get_env(current_text().c_str());
				if (tmp)
					result.m_args.emplace_back(tmp);
			}
			continue;
		}
		redirection();
	}
	m_programs_to_run.push_back(result);
}

void Parser::redirection() {
	assert(0);
}

void Parser::main_loop() {
	if (m_interactive) {
		while (!m_finish) {
			if (m_error) {
				m_error = false;
			}
			print_prompt();
			root();
		}
	}
	else {
		root();
	}
}

void Parser::print_prompt() {
	std::string working_directory = get_wd();
	std::string user_name = get_login();
	std::string home_directory = get_hd();

	// make relative path
	auto pos = working_directory.find(home_directory);
	if (pos != std::string::npos) {
		working_directory.erase(pos, home_directory.size());
		working_directory = '~' + working_directory;
	}

	std::cout << user_name << ':' << working_directory << "$ ";
}

void Parser::execute_to_run() {
	if (m_error) {
		m_programs_to_run.clear();
		return;
	}
	for (auto&& iter : m_programs_to_run) {
		auto args = iter.get_argv();
		iter.m_pid = execute(iter.m_name.c_str(), args.get());
	}

	for (auto&& iter : m_programs_to_run)
		m_last_return_value = wait_for_process(iter.m_pid);

	m_programs_to_run.clear();
}

int Parser::last_return_value() {
	return m_last_return_value;
}


