#include "pch.h"
#include "tasks.h"

namespace mob::tasks
{

zlib::zlib()
	: basic_task("zlib")
{
}

std::string zlib::version()
{
	return conf().version().get("zlib");
}

bool zlib::prebuilt()
{
	return false;
}

fs::path zlib::source_path()
{
	return conf().path().build() / ("zlib-" + version());
}

void zlib::do_clean(clean c)
{
	if (is_set(c, clean::redownload))
		run_tool(downloader(source_url(), downloader::clean));

	if (is_set(c, clean::reextract))
	{
		cx().trace(context::reextract, "deleting {}", source_path());
		op::delete_directory(cx(), source_path(), op::optional);
		return;
	}

	if (is_set(c, clean::reconfigure))
		run_tool(create_cmake_tool(cmake::clean));

	if (is_set(c, clean::rebuild))
		run_tool(create_msbuild_tool(msbuild::clean));
}

void zlib::do_fetch()
{
	const auto file = run_tool(downloader(source_url()));

	run_tool(extractor()
		.file(file)
		.output(source_path()));
}

void zlib::do_build_and_install()
{
	const fs::path build_path = run_tool(create_cmake_tool());

	run_tool(create_msbuild_tool());

	op::copy_file_to_dir_if_better(cx(),
		build_path / "zconf.h",
		source_path());
}

cmake zlib::create_cmake_tool(cmake::ops o)
{
	return std::move(cmake(o)
		.generator(cmake::vs)
		.root(source_path())
		.prefix(source_path()));
}

msbuild zlib::create_msbuild_tool(msbuild::ops o)
{
	const fs::path build_path = create_cmake_tool().build_path();

	return std::move(msbuild(o)
		.solution(build_path / "INSTALL.vcxproj"));
}

url zlib::source_url()
{
	return "https://zlib.net/zlib-" + version() + ".tar.gz";
}

}	// namespace
