#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include <string>
#include <vector>
#include <tuple>
#include <memory>

#include <cstring>

struct Fdredirection {
	Fdredirection() {
		from = 0;
		to = 0;
	}
	int from;
	int to;
};

struct Nfredirection {
	Nfredirection() {
		from = 0;
		append = false;
	}
	int from;
	std::string to;
	bool append;
};

struct Program {
	std::string m_name;
	std::vector<std::string> m_args;
	std::vector<Fdredirection> m_fd;
	std::vector<Nfredirection> m_nf;

	bool m_pipe;
	int m_pid;

	Program() {
		m_pid = 0;
		m_pipe = false;
	}

	auto get_argv() {
		auto deleter = [](char** args) {
			for (int iter = 0; args[iter]; ++iter)
				delete[] args[iter];

			delete[] args;
		};

		int arg_count = m_args.size() + 1; // 1 for program name
		char** args = new char*[arg_count + 1];

		// preparing program name
		args[0] = new char[m_name.size() + 1];
		std::strcpy(args[0], m_name.c_str());

		for (int iter = 1; iter < arg_count; ++iter) {
			args[iter] = new char[m_args[iter - 1].size()];
			std::strcpy(args[iter], m_args[iter - 1].c_str());
		}

		args[arg_count] = nullptr;

		return std::unique_ptr<char*, decltype(deleter)>(args, deleter);
	}
};

#endif
