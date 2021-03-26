#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include <string>
#include <vector>
#include <tuple>
#include <memory>

#include <cstring>

#include "simple_structs.h"

struct Fdredirection {
	Fdredirection(int from = 0, int to = 0) : from(from), to(to) { }
	int from;
	int to;
};

struct Nfredirection {
	Nfredirection(int from = 0, std::string to = std::string(), bool append = false) : from(from), to(to), append(append) { }
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

	auto get_fd_redirections() {
		auto result = new Fdsimple[m_fd.size() + 1];
		*reinterpret_cast<int*>(result) = m_fd.size();
		for (int iter = 0; iter < m_fd.size(); ++iter) {
			result[iter + 1].from = m_fd[iter].from;
			result[iter + 1].to = m_fd[iter].to;
		}
		return std::unique_ptr<Fdsimple>(result);
	}

	auto get_nf_redirections() {
		auto result = new Nfsimple[m_nf.size() + 1];
		*reinterpret_cast<int*>(result) = m_nf.size();
		for (int iter = 0; iter < m_nf.size(); ++iter) {
			result[iter + 1].from = m_nf[iter].from;
			result[iter + 1].to = m_nf[iter].to.c_str();
			result[iter + 1].append = m_nf[iter].append;
		}
		return std::unique_ptr<Nfsimple>(result);

	}
};

#endif
