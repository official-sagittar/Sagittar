workspace "Sagittar"
    configurations { "Debug", "Test", "Release" }
    platforms { "macos64", "linux64", "windows64" }

project "Sagittar"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    targetname ("%{prj.name}-" .. os.target() .. "-")
    targetdir "bin/%{cfg.buildcfg}"
    entrypoint ("main()")
    files { "src/**.h", "src/**.cpp" }
    pchheader "src/commons/pch.h"
    pchsource "src/commons/pch.cpp"
    includedirs { "src" }
    warnings "Extra"
    buildoptions { "-march=native" }
    linkoptions { "-static" }
    linkerfatalwarnings { "warnings" }

    newoption {
        trigger = "enable-san",
        description = "Enable ASan and UBSan"
    }

    filter { "options:enable-san" }
        buildoptions { "-march=native", "-fsanitize=undefined", "-fsanitize=address" }
        linkoptions { "-static", "-fsanitize=undefined", "-fsanitize=address" }

    filter { "platforms:macos64" }
        system "macosx"
        architecture "x86_64"
        removelinkoptions { "-static" }
        toolset ("clang")
        targetsuffix ("x86_64")

    filter { "platforms:linux64" }
        system "linux"
        architecture "x86_64"
        toolset ("gcc")
        targetsuffix ("x86_64")

    filter { "platforms:windows64" }
        system "windows"
        architecture "x86_64"
        toolset ("gcc")
        targetsuffix ("x86_64")

    filter { "configurations:Debug" }
        defines { "DEBUG" }
        symbols "On"
        linktimeoptimization "Off"

    filter { "configurations:Test" }
        files { "src/**.h", "src/**.cpp", "test/*.h", "test/*.cpp" }
        removefiles { "src/sagittar/main.cpp" }
        includedirs { "src",  "test/lib/doctest" }
        defines { "DEBUG", "TEST" }
        linktimeoptimization "On"
        optimize "Speed"

    filter { "configurations:Release" }
        defines { "NDEBUG" }
        fatalwarnings { "All" }
        linktimeoptimization "On"
        optimize "Speed"

newaction {
    trigger = "clean",
    description = "Remove all binaries and build folders.",
    execute = function()
        os.rmdir("./bin")
        os.rmdir("./obj")
    end
}
