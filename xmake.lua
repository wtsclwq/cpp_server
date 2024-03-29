set_project("cpp-server")
add_rules("mode.release", "mode.debug")
add_requires("boost","yaml-cpp","openssl")
add_packages("boost","yaml-cpp")
set_languages("c++17")
set_toolchains("clang")

target("util")
    set_kind("shared")
    add_files("src/util/*.cpp")

target("concurrency")
    set_kind("shared")
    add_files("src/concurrency/*.cpp")

target("config")
    set_kind("shared")
    add_files("src/config/*.cpp")

target("log")
    set_kind("shared")
    add_files("src/log/*.cpp")

target("config")
    set_kind("shared")
    add_files("src/config/*.cpp")

target("timer")
    set_kind("shared")
    add_files("src/timer/*.cpp")

target("io")
    set_kind("shared")
    add_files("src/io/*.cpp")

target("serialize")
    set_kind("shared")
    add_files("src/serialize/*.cpp")

target("socket")
    set_kind("shared")
    add_files("src/socket/*.cpp")
    add_packages("openssl")

target("http")
    set_kind("shared")
    add_files("src/http/*.cpp")

target("server")
    set_kind("shared")
    add_files("src/server/*.cpp")

target("concurrency_test")
    set_kind("binary")
    add_files("test/concurrency_test.cpp")
    add_deps("log","util","config","concurrency","io","timer")

target("io_manager_test")
    set_kind("binary")
    add_files("test/io_manager_test.cpp")
    add_deps("log","util","config","concurrency","timer","io")

target("address_test")
    set_kind("binary")
    add_files("test/address_test.cpp")
    add_deps("socket","log","util","config","concurrency","timer","io")

target("socket_test")
    set_kind("binary")
    add_files("test/socket_test.cpp")
    add_deps("socket","log","util","config","concurrency","timer","io","serialize")

target("serialize_test")
    set_kind("binary")
    add_files("test/serialize_test.cpp")
    add_deps("serialize","socket","log","util","config","concurrency","timer","io")

target("http_test")
    set_kind("binary")
    add_files("test/http_test.cpp")
    add_deps("http","serialize","socket","log","util","config","concurrency","timer","io")

target("parser_test")
    set_kind("binary")
    add_files("test/parser_test.cpp")
    add_deps("http","serialize","socket","log","util","config","concurrency","timer","io")

target("tcp_server_test")
    set_kind("binary")
    add_files("test/tcp_server_test.cpp")
    add_deps("server","http","serialize","socket","log","util","config","concurrency","timer","io")

target("tcp_client_test")
    set_kind("binary")
    add_files("test/tcp_client_test.cpp")
    add_deps("server","http","serialize","socket","log","util","config","concurrency","timer","io")


--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro defination
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--

