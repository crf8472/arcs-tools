\mainpage ARCS Tools - An AccurateRip Checksums ToolKit


\section Home

See the project home [on codeberg.org](https://codeberg.org/tristero/arcst-tools)


\section features Features

Arcs-tools provide an example implementation of the libarcstk feature set. The
toolkit supports the following tasks for working with AccurateRip checksums:

- Calculate the AccurateRip checksums of a succession of audio samples, may it
  be a single track or an album
- Verify AccurateRip checksums against their reference sums from AccurateRip
- Calculate the AccurateRip ID of a CD from its TOC information (along with the
  request URL and the canonical response filename)
- Parse the response of an request to the AccurateRip database to plaintext



\section nonfeatures Non-features

- No ripping of CDs
- Does not send any requests to AccurateRip. Has no network facilities and is
  not supposed to get any. The actual HTTP request for fetching the reference
  values is better performed by the HTTP client of your choice. (Examples are
  provided [in the corresponding README section](../../README.md).)



\section howtobuild How to Build

Build and install to just use the API:

	$ cd arcs-tools      # your project root directory where README.md resides
	$ mkdir build && cd build
	$ cmake -DCMAKE_BUILD_TYPE=Release ..
	$ cmake --build .
	$ sudo make install  # installs to /usr/local

Note that arcs-tools need libarcstk and libarcsdec to compile. If the first
cmake call issues an error that reads "Could NOT find libarcsdec", check whether
they are properly installed. Locally, I deploy these libs to /usr/local. My
CMake will not find them unless the path is explicitly specified as part of the
prefix:

	$ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/usr/local ..
