# How to Build



## Building arcs-tools on Linux and \*BSD

Arcs-tools are written in C++17. The project was developed mainly (but not
exclusively) for Linux. Arcs-tools have two dependencies, [libarcstk][5] and
[libarcsdec][6] where the latter may pull in more dependencies like libcue,
FLAC, libsndfile, ffmpeg and others if configured to do so. It was not tested
whether arcs-tools builds out-of-the-box on BSDs but don't expect major issues.



## Buildtime dependencies


### Mandatory Buildtime Dependencies

- C++-17-compliant-compiler with C++ standard library
- ``cmake`` >= 3.9.6
- ``make`` or some other build tool compatible to cmake (the examples suppose
  ``make`` nonetheless)

### Optional Buildtime Dependencies

- git - for testing: to clone test framework [Catch2][2] as an external project
  when running the unit tests. For building the documentation with
  [m.css][3] (instead of stock doxygen) to clone m.css.
- Doxygen - for documentation: to build the API documentation in HTML
  (graphviz/dot is not required)
- Python (with ``virtualenv``) - for documentation: to build the documentation
  in HTML styled with [m.css][3]
- LaTeX (TeXLive for instance) - for documentation: to build the documentation
  in LaTeX


## Building the executable without system-wide installation

We presuppose you have downloaded and unpacked or git-cloned arcs-tools to a
folder named ``arcs-tools``. Then, for just testing arcs-tools, do:

	$ cd arcs-tools      # your arcs-tools root folder where README.md resides

If you are interested in a particular branch, check this branch out now:

	$ git checkout $BRANCHNAME

If you want to build from ``main`` you do not need to checkout any branch.

	$ git submodule init
	$ git submodule update     # pull libarcstk+libarcsdec in build-tree
	$ mkdir build && cd build  # create build folder for out-of-source-build
	$ cmake -DWITH_SUBMODULES=ON ..
	$ cmake --build .          # perform the actual build

This will checkout libarcstk and libarcsdec into the local build-tree. When
building, the binary ``arcstk`` is created linked to those locally compiled
dependencies with all optimizations and without debug-symbols and tests. Note
that this process will only succeed if all default dependencies of libarcsdec
are available! You can create ``./libs/libarcsdec/build`` and configure
libarcsdec there for the set of dependencies you prefer and redo the build.

Do not activate ``-DWITH_SUBMODULES`` if you plan to install the binary in your
local OS-tree!  You might experience problems when loading the shared objects!
The submodule-based approach is for development only: it targets a situation
where you just need to recompile to test something without having to manage all
dependencies!

We describe the build configuration for the following profiles:
- [User](#users) (read: a person who uses arcs-tools on her plattform)
- [Contributing developer](#contributors) (a person who wants to debug and test
  arcs-tools)
- [Package maintainer](#package-maintainers) (a person who intends to package
  arcs-tools for some target system).


### Using a different compiler

Arcs-tools come tested to compile with clang++ as well as with g++.

If you have both and want to switch the compiler, you should just hint CMake
what compiler to use. On many unixoid systems you can do this via the
environment variables ``CC`` and ``CXX``.

If your actual compiler is not clang and you want to use your installed clang:

	$ export CC=$(type -p clang)
	$ export CXX=$(type -p clang++)

If your actual compiler is not g++ and you want to use your installed g++:

	$ export CC=$(type -p gcc)
	$ export CXX=$(type -p g++)

Then, delete your directory ``build`` (which contains metadata from
the previous compiler) to start off cleanly.

	$ cd ..
	$ rm -rf build

CMake-reconfigure the project to have the change take effect:

	$ mkdir build && cd build
	$ cmake ..

During the configure step, CMake informs about the actual C++-compiler like:

	-- The CXX compiler identification is Clang 10.0.0
	...
	-- Check for working CXX compiler: /usr/bin/clang++ - works


### Users

You intend to install arcs-tools on your system to perform tasks using them.
You just want arcs-tools to be available in your path and not getting in your
way:

	$ cmake -DCMAKE_BUILD_TYPE=Release ..
	$ cmake --build .
	$ sudo make install

This will install the following files to your system:

- the binary ``arcstk`` (along with four symbolic links starting with
  ``arcstk-``) in the local binary location (e.g. ``/usr/local/bin``)
- 4 manpages for the 4 symbolic tools in the default manpage location (and there
  in ``man1``).

You can change the install location by calling cmake with the
``-DCMAKE_INSTALL_PREFIX=/path/to/install/dir`` switch.



### Contributors

You want to debug into the code, hence you need to build arcs-tools with
debugging symbols and without aggressive optimization:

	$ cmake -DCMAKE_BUILD_TYPE=Debug ..
	$ cmake --build .

For also building and running the tests, just use the corresponding switch:

	$ cmake -DCMAKE_BUILD_TYPE=Debug -DWITH_TESTS=ON ..
	$ cmake --build .
	$ ctest

Note: This build will take *significantly* *longer* than the build without
tests.

#### Turn optimizing on/off

You may or may not want the ``-march=native`` and ``-mtune=generic`` switches on
compilation. For Debug-builds, they are ``OFF`` by default, but can be added by
using ``-DWITH_NATIVE=ON``. For now, this switch has only influence when using
g++ or clang++. For other compilers, default settings apply.

#### Run unit tests

Note that ``-DWITH_TESTS=ON`` will try to git-clone the testing framework
[Catch2][2] within your ``build`` directory and fail if this does not work.

Running the unit tests is *not* part of the build process. To run the tests,
invoke ``ctest`` manually in the ``build`` directory after ``cmake --build .``
was completed successfully.

Note that ctest will write report files in the ``build`` folder, their name
pattern is ``report.<testcase>.xml`` where ``<testcase>`` corresponds to a
``.cpp``-file in ``test/src``.

#### Find unused header includes

From time to time, I tend to mess up the includes. So for mere personal use, I
use IWYU for identifying unused includes that can be removed.

CMake brings some support for Google's tool [include-what-you-use][1]. If you
have installed IWYU, you can just use CMake to call it on the project:

	$ cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_INCLUDE_WHAT_YOU_USE=$(type -p include-what-you-use) ..
	$ cmake --build . 2>iwuy.txt

This runs every source file through inlcude-what-you-use instead of the actual
compiler and writes the resulting analysis to file ``iwyu.txt``. If you want to
use your real compiler again, you have to reconfigure the project.

The tool may log some warnings about unknown compile switches when you have
selected g++ as your actual compiler. This is just because there are some
switches configured for your actual compiler that are unknown to the tool. The
warnings can be ignored. To avoid them
[switch to clang++](#using-a-different-compiler), then configure the project
with ``-DCMAKE_CXX_INCLUDE_WHAT_YOU_USE=...`` pointing to the IWYU-binary and
run the build again. (Note that if you have the binary in the path, you do not
need to specify a path. Otherwise, it is recommended to specify an absolute
path.)


### Package maintainers

You want to build arcs-tools with a release profile but without any architecture
specific optimization (e.g. without ``-march=native`` and ``-mtune=generic`` for
g++ or clang++).

Furthermore, you would like to adjust the install prefix path such that
arcs-tools is configured for being installed in the real system prefix (such as
``/usr``) instead of some default prefix (such as ``/usr/local``).

You may also want to specify a staging directory as an intermediate install
target.

When using clang or gcc, all of these can be achieved as follows:

	$ cmake -DCMAKE_BUILD_TYPE=Release -DWITH_NATIVE=OFF -DCMAKE_INSTALL_PREFIX=/usr ..
	$ cmake --build .
	$ make DESTDIR=/my/staging/dir install

*Note* that ``-DWITH_NATIVE=OFF`` currently only works for clang++ and g++.

If you use another compiler than clang++ or g++, CMake will not apply any
project specific modifications to the compiler default settings. Therefore, you
have to carefully inspect the build process (e.g. by using ``$ make VERBOSE=1``)
to verify which compiler settings are actually used.

By default, the release-build of arcs-tools uses ``-O3``. If you intend to
change that, locate the paragraph ``Compiler Specific Settings`` in
[CMakeLists.txt](./CMakeLists.txt) in the root directory and adjust the settings
to your requirements.


## Configure switches

|Switch              |Description                                     |Default|
|--------------------|------------------------------------------------|-------|
|CMAKE_BUILD_TYPE    |Build type for release or debug             |``Release``|
|CMAKE_INSTALL_PREFIX|Top-level install location prefix     |plattform defined|
|CMAKE_EXPORT_COMPILE_COMMANDS|Rebuilds a [compilation database](#deep-language-support-in-your-editor) when configuring |OFF    |
|WITH_DOCS           |Configure for [documentation](#building-the-api-documentation)                     |OFF    |
|WITH_NATIVE         |Use platform [specific optimization](#turn-optimizing-on-off) on compiling         |       |
|WITH_TESTS          |Compile [tests](#run-unit-tests) (but don't run them)                              |OFF    |
|MCSS                |[Use m.css](#doxygen-by-m-css-with-html5-and-css3-tested-but-still-experimental) when building the documentation.  |OFF    |



## Building the API documentation

When you configure the project, switch ``-DWITH_DOCS=ON`` is required to prepare
building the documentation. Only this configuration option will create the
target ``doc`` that can be used to build the documentation.

	$ cmake --build . --target doc

Doxygen is required for building the documentation.

The documentation can be build as a set of static HTML pages (recommended) or as
a PDF manual using LaTeX (experimental, very alpha).

If you decide to build HTML, you may choose either the stock HTML output of
doxygen or the HTML output styled by m.css. Doxygen's stock HTML output is
stable but may appear a little bit outdated. The m.css-styled output is much,
much more user-friendly, clean and fit for documentation of modern C++. On the
other hand it is more cutting edge and therefore maybe not as stable as
doxygen's stock HTML output. Credits for the amazing m.css tool go to
[mozra][3].


### Quickstart: Doxygen Stock HTML

The generation of the documentation sources must be requested at configuration
stage. The documentation sources will not be generated automatically during
build. It is required to call target ``doc`` manually.

	$ cd build
	$ cmake -DWITH_DOCS=ON ..
	$ cmake --build . --target doc

This will build the documentation sources for HTML as well as LaTeX in
subdirectories of ``build/generated-docs/``. Open the file
``build/generated-docs/html/index.html`` in your browser to see the entry page.


### Doxygen by m.css with HTML5 and CSS3 (tested, but still experimental)

Accompanying [m.css][3] comes a doxygen style. It processes the doxygen XML
output and generates a static site in plain HTML5 and CSS3 from it (nearly
without JavaScript).

The resulting site presents the documentation content very clean and
well structured, using a more contemporary design than the stock doxygen HTML
output. (Which, on the other hand, gives us this warm nostalgic memory of the
Nineties... we loved the Nineties, didn't we?)

The [public APIdoc of arcs-tools is build with m.css][4].

This APIdoc can be built locally by the following steps:

	$ cd build
	$ cmake -DWITH_DOCS=ON -DMCSS=ON ..
	$ cmake --build . --target doc

CMake then creates a local python sandbox in ``build`` with ``virtualenv``,
installs jinja2 and Pygments in it, then clones [m.css][3], and runs m.css
which internally runs doxygen. Maybe this process needs finetuning for some
environments I did not foresee. (It is completely untested on Windows and may
not work.)

Documentation is generated in ``build/generated-docs/mcss`` and you can
load ``build/generated-docs/mcss/html/index.html`` in your browser.

Note that ``-DMCSS=ON`` turns off the LaTeX output! You cannot generate
m.css and LaTeX output in the same build.


### Manual: PDF by LaTeX (smoke-tested, more or less)

Arcs-tools provides also support for a PDF manual using LaTeX. An actual LaTeX
installation (along with ``pdflatex``) is required for creating the manual.

Building the PDF manual is only available when ``MCSS`` is ``OFF``. Using
``-DMCSS=ON`` will effectively turn off LaTeX source generation! If you have
previously configured ``MCSS``, just reconfigure your build:

	$ cmake -DWITH_DOCS=ON -DMCSS=OFF ..

Building the ``doc`` target like in the examples above will create the LaTeX
sources for the PDF manual but will not automatically typeset the actual PDF
document. This requires to change directory to ``build/generated-docs/latex``
and issue ``make``.

The entire process:

	$ cd build
	$ cmake -DWITH_DOCS=ON ..  # Do not use -DMCSS=ON!
	$ cmake --build . --target doc
	$ cd generated-docs/latex
	$ make

This will create the manual ``refman.pdf`` in folder
``build/generated-docs/latex`` (while issueing loads of ``Underfull \hbox``
warnings, which is perfectly normal).

Note that I did never give any love to the LaTeX-based PDF manual. It will build
but it will not be convenient or look good at its current stage!


## Deep language support in your ``$EDITOR``

The project provides a workflow to create a compilation database as a basic
support for what is usually called "deep language support" (DLS).

You may have noticed that arcs-tools comes with a top-level ``.clang`` file that
already points to ``compile_commands.json`` in the same directory. This prepares
the support for clang-based DSL for arcs-tools, but you have to create the
compilation database on your own, for your compiler and your settings:

	$ cd build
	$ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
	$ cd ..
	$ ln -s build/compile_commands.json . # May or may not be required

Whenever the compilation process changes - say, a source file is added or
removed or the code changes significantly - you should recreate the compilation
database.

If this all sounds odd for you, it is completely safe to skip this paragraph,
ignore the ``.clang`` file and just feel good. It will not get in your way.


## Build on Windows ... duh!

No Windows port yet :-(

In fact, as a lack of requirement, arcs-tools has not yet even been tried to be
built on Windows.

To avoid any show-stoppers for porting arcs-tools to Windows or other platforms,
arcs-tools relies completely on pure C++14 and the C++ standard library. It does
not require any other dependencies. In fact, it is intended to not use platform
specific operations at all. Code that breaks platform independence will be
considered being a bug. The porting is expected not to be difficult, but is
just not done. Help will be appreciated.


[1]: https://include-what-you-use.org/
[2]: https://github.com/catchorg/Catch2
[3]: https://mcss.mosra.cz/doxygen/
[4]: https://crf8472.github.io/arcs-tools/current/
[5]: https://github.com/crf8472/libarcstk
[6]: https://github.com/crf8472/libarcsdec

