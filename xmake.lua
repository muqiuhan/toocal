add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "."})

includes("./libs")

add_requires(
    "spdlog",
    "tl_expected",
    "tl_optional",
    "fmt",
    "endian"
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

for _, file in ipairs(os.files("tests/core/test_*.cpp")) do
     local name = path.basename(file)
     target(name)
         set_kind("binary")
         set_default(false)
         set_languages("c++20")

         add_files("tests/core/" .. name .. ".cpp")
         add_tests("default")
         add_includedirs("core")
         add_deps("toocal_core")
         add_packages(
            "spdlog",
            "tl_expected",
            "tl_optional",
            "fmt",
            "endian"
         )
end