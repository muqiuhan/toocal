add_rules("mode.debug", "mode.release")

target("toocal")
    set_kind("binary")
    set_languages("c++20")
    add_files("src/*.cpp")