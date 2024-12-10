add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "."})

includes("./libs")

add_requires(
    "spdlog",
    "tl_expected",
    "tl_optional",
    "endian",
    "doctest"
)
    
target("toocal")
    set_kind("binary")
    set_languages("c++20")
    add_files("core/*.cpp", "cli/*.cpp")
    add_includedirs("core")
    add_packages(
        "spdlog",
        "tl_expected",
        "tl_optional",
        "endian"
    )