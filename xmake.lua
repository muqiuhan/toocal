add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "."})

includes("./libs")

add_requires(
    "spdlog",
    "tl_expected",
    "tl_optional",
    "endian",
    "doctest",
    "mimalloc"
)
    
target("toocal_core")
    set_kind("static")
    set_languages("c++20")
    add_files("core/*.cpp")
    add_includedirs("libs")
    add_packages(
        "spdlog",
        "tl_expected",
        "tl_optional",
        "endian",
        "mimalloc"
    )
    add_links("mimalloc")

target("toocal")
    set_kind("binary")
    set_languages("c++20")
    add_files("cli/*.cpp")
    add_includedirs("core", "libs")
    add_deps("toocal_core")
    add_packages(
        "spdlog",
        "tl_expected",
        "tl_optional",
        "endian"
    )
    add_links("toocal_core", "mimalloc")

for _, file in ipairs(os.files("tests/test_*.cpp")) do
     local name = path.basename(file)
     target(name)
        set_kind("binary")
        set_default(false)
        set_languages("c++20")
        add_files("tests/" .. name .. ".cpp")
        add_tests(name)
        add_deps("toocal_core")
        add_includedirs("core", "libs")
        add_packages(
            "spdlog",
            "tl_expected",
            "tl_optional",
            "endian"
        )
        add_links("mimalloc")

end