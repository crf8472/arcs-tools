#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_TOOLS_FS_HPP__
#include "tools-fs.hpp"
#endif


TEST_CASE ( "path()", "" )
{
	using arcsapp::file::path;

	CHECK ( path("foo/bar/baz/myfile.txt") == "foo/bar/baz/" );
	CHECK ( path("foo/myfile.txt")         == "foo/" );

	CHECK ( path("/foo/bar/baz/myfile.txt") == "/foo/bar/baz/" );
	CHECK ( path("/foo/myfile.txt")         == "/foo/" );

	CHECK ( path("file_with_no_path.flac")  == "" );

	CHECK ( path("")                        == "" );
}


TEST_CASE ( "prepend_path()", "" )
{
	using arcsapp::file::prepend_path;

	auto filename = std::string { "foo.txt" };
	prepend_path("foo/bar/", filename);

	CHECK ( filename == "foo/bar/foo.txt" );

	filename = "without_suffix";
	prepend_path("/some/absolute/path/", filename);

	CHECK ( filename == "/some/absolute/path/without_suffix" );

	filename = "without_suffix";
	prepend_path("", filename);

	CHECK ( filename == "without_suffix" );

	filename = "";
	prepend_path("/some/path/", filename);

	CHECK ( filename == "/some/path/" );

	filename = "";
	prepend_path("", filename);

	CHECK ( filename == "" );
	CHECK ( filename.empty() );
}

