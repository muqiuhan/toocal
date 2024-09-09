add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "."})

includes("./libs")

add_requires(
    "spdlog",
    "tl_expected",
    "tl_optional",
    "fmt",
    "endian",
    "doctest"
)
    
target("toocal_core")
    set_kind("binary")
    set_languages("c++20")
    add_files("core/*.cpp")
    add_packages(
        "spdlog",
        "tl_expected",
        "tl_optional",
        "fmt",
        "endian"
    )

target("test_toocal_core")
     set_kind("binary")
     set_default(false)
     set_languages("c++20")

     add_includedirs("core")
     add_files("core/*.cpp")
     for _, testfile in ipairs(os.files("tests/core/*.cpp")) do
         add_tests(path.basename(testfile), {
             files = testfile,
             remove_files = "core/main.cpp",
             languages = "c++20",
             packages = "doctest",
             defines = "DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN"})
     end
     add_packages(
        "spdlog",
        "tl_expected",
        "tl_optional",
        "fmt",
        "endian"
     )