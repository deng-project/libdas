--- libdas: DENG asset management library
--- licence: Apache, see LICENCE file
--- file: premake5.lua - Main premake5 configuration file
--- author: Karl-Mihkel Ott


workspace "libdas"
    cleancommands { "make clean %{cfg.buildcfg}" }
    configurations { "Debug", "Release" }
    platforms { "Win32", "Linux" }
    includedirs { "./include" }

    architecture "x86_64"
    pic "On"

    -- Ignore safety warnings that MSVC gives
    filter "platforms:Win32"
        defines { "_CRT_SECURE_NO_WARNINGS" }

    -- Enable debug symbols if requested
    filter "configurations:Debug"
        symbols "On"
        optimize "Off"
        targetdir "build/debug"
        defines { "_DEBUG" }

    filter "configurations:Release"
        symbols "Off"
        optimize "Speed"
        targetdir "build/release"

    filter {}

--- Add testing flags ---
newoption {
    trigger = "tests",
    value = "CONF",
    default = "none",
    description = "Specify the test application building",
    allowed = {
        { "none", "Do not build any unit tests (default)" },
        { "AsciiStreamReader", "Build AsciiStreamReader class test" },
        { "WavefrontObjParser", "Build WavefrontObjParser class test" },
        { "AsciiSTLParser", "Build AsciiSTLParser class test" },
        { "BinarySTLParser", "Build BinarySTLParser class test" },
        -- tmp --
        { "WriterCore", "Build DasWriterCore class test" },
        { "MatrixTest", "Build Matrix test application" },

        { "all", "Build all available unit tests" }
    }
}


--- Add option to avoid building the library ---
newoption {
    trigger = "no-lib",
    description = "Do not build libdas library"
}


-- Define an option to clean --
cleancommands {
    "make clean %{cfg.buildcfg}"
}



-- Check if given operating system is supported --
function OsCheck()
    if not os.istarget("linux") and not os.istarget("windows") then
        local host = os.target()
        host = host:gsub("%a", string.upper, 1)
        print(host .. " is not supported by Libdas :(")
        os.exit()
    end
end


-- Check which test configs to include --
function LoadTests() 
    -- Test options checking
    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "AsciiStreamReader" then
        print("Including AsciiStreamReader test config")
        local stream_reader = require("premake/tests/AsciiStreamReader")
        stream_reader.build()
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "WavefrontObjParser" then
        print("Including WavefrontObjParser test config")
        local parser = require("premake/tests/WavefrontObjParser")
        parser.build()
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "AsciiSTLParser" then
        print("Including AsciiSTLParser test config")
        local parser = require("premake/tests/AsciiSTLParser")
        parser.build()
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "BinarySTLParser" then
        print("Including BinarySTLParser test config")
        local parser = require("premake/tests/BinarySTLParser")
        parser.build()
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "WriterCore" then
        print("Including WriterCore test config")
        local writer = require("premake/tests/WriterCore")
        writer.build()
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "Matrix" then
        print("Including Matrix classes test config")
        local mat = require("premake/tests/Matrix")
        mat.build()
    end
end


-- Load all required build configs --
function LoadBuildConfigs()
    -- Load libdas build configuration --
    if not _OPTIONS["no-lib"] then
        print("Including libdas configurations")
        local libdas = require("premake/libdas")
        libdas.build()
    end

    LoadTests()
end


if not _OPTIONS["help"] then
    OsCheck()
    LoadBuildConfigs()
end
