#include "parser.hpp"
#include "low_level.h"
#include <iostream>
#include <fstream>

Parser::Parser(int argc, const char* const argv[]) :
		m_lex(argc == 1),
		m_interactive(argc == 1) {
	m_token_stack_position = -1;
	m_finish = false;
	m_last_return_value = 0;
	m_error = false;

	// preparing stream
	if (m_interactive)
		m_lex.set_stream(&std::cin);
	else {
		m_stream = std::make_unique<std::ifstream>(argv[1]);
		m_lex.set_stream(m_stream.get());
	}


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
			if (last_return_value() != 0) {
				skip_to_separator();
				break;
			}
		}
		if (accept(_OROR)) {
			execute_to_run();
			if (last_return_value() == 0) {
				skip_to_separator();
				break;
			}
		}
	}
	execute_to_run();
}

void Parser::pipeline() {
	command();
	while (accept(_PIPE)) {
		if (m_error)
			return;
		m_programs_to_run.back().m_pipe = true;
		command();
	}
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
		redirection(result);
	}
	if (!result.m_name.empty())
		m_programs_to_run.push_back(result);
}

void Parser::redirection(Program& pr) {
	int fildes = -1;
	std::string name;
	bool append = false;

	if (accept(_FILDES))
		fildes = std::stoi(current_text());
	else if (accept(_ARGUMENT)) {
		name = current_text();
		assert(0); // not supported yet
	}


	if (accept(_OUT_REDIRECTION)) {
		if (fildes == -1)
			fildes = 1;
	}
	else if (accept(_IN_REDIRECTION)) {
		if (fildes == -1)
			fildes = 0;
	}
	else if (accept(_APPEND_REDIRECTION)) {
		if (fildes == -1)
			fildes = 1;
		append = true;
	}


	if (accept(_ARGUMENT)) {
		pr.m_nf.emplace_back(
				fildes, 
				current_text(), 
				append);
	}

	else if (accept(_FILDES)) {
		pr.m_fd.emplace_back(
				fildes, 
				std::stoi(current_text()));
	}
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
	if (m_programs_to_run.empty())
		return;
	if (m_error) {
		m_programs_to_run.clear();
		return;
	}
	int pipes[2] = {0, 0};
	int pipe_for_child[2] = {0, 0};
	for (auto&& iter : m_programs_to_run) {
		auto args = iter.get_argv();
		if (iter.m_pipe) {
			open_pipe(pipes);
			pipe_for_child[1] = pipes[1];
		}
		else {
			pipe_for_child[1] = 0;
		}
		set_pipe_for_child(pipe_for_child);

		auto fd_redirections = iter.get_fd_redirections();
		auto nf_redirections = iter.get_nf_redirections();

		iter.m_pid = execute(
				iter.m_name.c_str(),
				args.get(),
				fd_redirections.get(),
				nf_redirections.get());

		if (iter.m_pipe) {
			pipe_for_child[0] = pipes[0];
		}
		else {
			pipe_for_child[0] = 0;
		}
	}

	for (auto&& iter : m_programs_to_run) {
		if (iter.m_pid > 0)
			m_last_return_value = wait_for_process(iter.m_pid);
	}

	m_programs_to_run.clear();
}

void Parser::skip_to_separator() {
	while (current_token() != _SEPARATOR && current_token() != _EOF)
		next_token();
}

int Parser::last_return_value() {
	return std::stoi(get_env("?"));
}


