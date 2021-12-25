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
        { "WavefrontObjParser", "Build WavefrontObjParser class test" },
        { "AsciiSTLParser", "Build AsciiSTLParser class test" },
        { "BinarySTLParser", "Build BinarySTLParser class test" },
        { "HuffmanCompression", "Build Huffman compression class test application" },
        { "WavefrontObjCompiler", "Build a Wavefront OBJ compiler test application" },
        { "STLCompiler", "Build an STL compiler test application" },
        { "Base64Decoder", "Build a Base64Decoder class test application" },
        { "JSONParser", "Build a JSONParser class test application" },
        { "GLTFParser", "Build a GLTFParser class test application" },
        -- tmp --
        { "TextureReader", "Build TextureReader class test" },
        { "DasReaderCoreTest", "Build DasReaderCore class test" },
        { "SubstringSearchTest", "Build substring search test programm" },

        -- misc --
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

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "HuffmanCompression" then
        print("Including HuffmanCompression test config");
        local huf = require("premake/tests/HuffmanCompression");
        huf.build()
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "WavefrontObjCompiler" then
        print("Including WavefrontObjCompiler test config")
        local wobj_compiler = require("premake/tests/WavefrontObjCompiler")
        wobj_compiler.build()
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "STLCompiler" then
        print("Including STLCompiler test config")
        local stl_compiler = require("premake/tests/STLCompiler")
        stl_compiler.build()
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "Base64Decoder" then
        print("Including Base64Decoder test config")
        local decoder = require("premake/tests/Base64Decoder")
        decoder.build()
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "JSONParser" then
        print("Including JSONParser test config")
        local parser = require("premake/tests/JSONParser")
        parser.build()
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "GLTFParser" then
        print("Including GLTFParser test config")
        local parser = require("premake/tests/GLTFParser")
        parser.build()
    end

    --if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "DasParser" then
        --print("Including DasParser test config")
        --local das_parser = require("premake/tests/DasParser")
        --das_parser.build()
    --end

    -- tmp --
    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "TextureReader" then
        print("Including TextureReader test config");
        local texture = require("premake/tests/TextureReader");
        texture.build()
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "DasReaderCore" then
        print("Including DasReaderCore test config")
        local reader = require("premake/tests/DasReaderCore")
        reader.build();
    end

    if _OPTIONS["tests"] == "all" or _OPTIONS["tests"] == "SubstringSearchTest" then
        print("Including SubstringSearchTest test config")
        local substr = require("premake/tests/SubstringSearchTest")
        substr.build()
    end
end


-- Load DASTool load configuration --
function LoadToolConfig() 
    local tool = require("premake/DASTool")
    tool.build(not _OPTIONS["no-lib"]);
end


-- Load all required build configs --
function LoadBuildConfigs()
    -- Load libdas build configuration --
    if not _OPTIONS["no-lib"] then
        print("Including libdas configurations")
        local libdas = require("premake/libdas")
        libdas.build()
        LoadTests()
    end

    LoadToolConfig();
end


if not _OPTIONS["help"] then
    OsCheck()
    LoadBuildConfigs()
end
