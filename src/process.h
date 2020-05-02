#pragma once

#include "utility.h"
#include "env.h"

namespace builder
{

class process
{
public:
	enum flags_t
	{
		noflags = 0x00,
		stdout_is_verbose = 0x01,
		allow_failure = 0x02
	};

	enum arg_flags
	{
		noargflags = 0x00,
		quiet   = 0x01,
		nospace = 0x02,
		quote   = 0x04
	};


	process();
	~process();

	static process raw(const std::string& cmd);

	static process pipe(const process& p)
	{
		return p;
	}

	template <class... Processes>
	static process pipe(const process& p1, const process& p2, Processes&&... ps)
	{
		auto r = p1;
		r.pipe_into(p2);
		pipe(r, std::forward<Processes>(ps)...);
		return r;
	}

	process& name(const std::string& name);
	const std::string& name() const;

	process& binary(const fs::path& p);
	const fs::path& binary() const;

	process& cwd(const fs::path& p);
	const fs::path& cwd() const;

	process& flags(flags_t f);
	flags_t flags() const;

	template <class T, class=std::enable_if_t<!std::is_same_v<T, arg_flags>>>
	process& arg(const T& value, arg_flags f=noargflags)
	{
		add_arg("", arg_to_string(value, (f & quote)), f);
		return *this;
	}

	template <class T, class=std::enable_if_t<!std::is_same_v<T, arg_flags>>>
	process& arg(const std::string& name, const T& value, arg_flags f=noargflags)
	{
		add_arg(name, arg_to_string(value, (f & quote)), f);
		return *this;
	}

	template <template<class, class> class Container, class K, class V, class Alloc>
	process& args(const Container<std::pair<K, V>, Alloc>& v, arg_flags f=noargflags)
	{
		for (auto&& [name, value] : v)
			arg(name, value, f);

		return *this;
	}

	template <class Container>
	process& args(const Container& v, arg_flags f=noargflags)
	{
		for (auto&& e : v)
			add_arg(e, "", f);

		return *this;
	}

	process& env(const builder::env& e);

	void run();
	void interrupt();
	void join();

	int exit_code() const;

private:
	struct impl
	{
		handle_ptr handle;
		std::atomic<bool> interrupt{false};

		impl() = default;
		impl(const impl&);
		impl& operator=(const impl&);
	};

	std::string name_;
	fs::path bin_;
	fs::path cwd_;
	flags_t flags_;
	builder::env env_;
	std::string raw_;
	std::string cmd_;

	impl impl_;
	DWORD code_;

	std::string make_name() const;
	std::string make_cmd() const;
	void pipe_into(const process& p);
	void do_run(const std::string& what);

	void add_arg(const std::string& k, const std::string& v, arg_flags f);

	std::string arg_to_string(const char* s, bool force_quote);
	std::string arg_to_string(const std::string& s, bool force_quote);
	std::string arg_to_string(const fs::path& p, bool force_quote);
	std::string arg_to_string(const url& u, bool force_quote);
};

}	// namespace
