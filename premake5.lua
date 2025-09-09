workspace "Sagittar"
    configurations { "Debug", "Test", "Release" }
    platforms { "macos64", "linux64", "windows64" }

    project "Sagittar"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++20"
        targetname "%{prj.name}"
        targetdir "bin/%{cfg.buildcfg}"
        entrypoint ("main()")
        files { "src/**.h", "src/**.cpp" }
        pchheader "src/pch.h"
        pchsource "src/pch.cpp"
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

        filter { "configurations:Debug" }
            defines { "DEBUG" }
            symbols "On"

        filter { "configurations:Test" }
            files { "src/**.h", "src/**.cpp", "test/*.h", "test/*.cpp" }
            removefiles { "src/main.cpp" }
            includedirs { "src",  "test/lib/doctest" }
            defines { "DEBUG", "TEST" }

        filter { "configurations:Release" }
            defines { "NDEBUG" }
            optimize "Speed"
            linktimeoptimization "On"

        filter { "platforms:macos64" }
            system "macosx"
            architecture "x86_64"
            toolset ("clang")
            removelinkoptions { "-static" }

        filter { "platforms:linux64" }
            system "linux"
            architecture "x86_64"
            toolset ("gcc")

        filter { "platforms:windows64" }
            system "windows"
            architecture "x86_64"
            toolset ("gcc")
