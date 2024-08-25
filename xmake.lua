add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "."})

add_requires("spdlog", "tl_expected", "tl_optional", "fmt")

target("toocal")
    set_kind("binary")
    set_languages("c++20")
    add_files("core/*.cpp")
    add_packages("spdlog", "tl_expected", "tl_optional", "fmt")