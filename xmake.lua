set_project("toocal")
set_version("0.0.1")
set_xmakever("2.8.1")

add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "."})

add_requires("plog")

target("toocal")
    set_kind("binary")
    set_languages("c++20")
    
    add_files("src/*.cpp")
    add_packages("plog")