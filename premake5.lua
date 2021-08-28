workspace "netlemon"
    startproject "netlemoneditor"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release"
    }
    
-- Bin / Bin-obj directory paths defined as global variables
tdir = "bin/%{cfg.buildcfg}/%{prj.name}"
odir = "bin-obj/%{cfg.buildcfg}/%{prj.name}"

-- Define all external dependencies
externals = {}
externals["asio"] = "external/asio-1.18.2"

project "netlemon"
    location "netlemon"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir(tdir)
    objdir(odir)

    files
    {
        "%{prj.name}/include/**.h",
        "%{prj.name}/src/**.h", 
        "%{prj.name}/src/**.cpp"
    }

    sysincludedirs
    {
        "%{prj.name}/include/netlemon",
    }

    flags { "FatalWarnings" }

    -- Windows
    filter {"system:windows", "configurations:*"}
        systemversion "latest" -- target latest windows upgrade when "cli gensln"
        defines{ "LEMON_PLATFORM_WINDOWS" }

    -- [CONFIGURATION DEFINES]: usually used to turn on optimization
    filter "configurations:Debug"
        defines "LEMON_CONFIG_DEBUG"
        runtime "Debug"
        symbols "on" -- pdb symbols / for release we don't want them

    filter "configurations:Release"
        defines "LEMON_CONFIG_RELEASE"
        runtime "Release"
        symbols "off"
        optimize "on"
        
project "netlemoneditor"
    location "netlemoneditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    links "netlemon"

    targetdir(tdir)
    objdir(odir)

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    sysincludedirs 
    { 
        "netlemon/include",
        "%{externals.asio}/include"
    }

    flags { "FatalWarnings" } 

    filter {"system:windows", "configurations:*"}
        systemversion "latest" -- target latest windows upgrade when "cli gensln"
        defines{ "LEMON_PLATFORM_WINDOWS" }

    -- [CONFIGURATION DEFINES]: usually used to turn on optimization
    filter "configurations:Debug"
        defines "LEMON_CONFIG_DEBUG"
        runtime "Debug"
        symbols "on" -- pdb symbols / for release we don't want them

    filter "configurations:Release"
        defines "LEMON_CONFIG_RELEASE"
        runtime "Release"
        symbols "off"
        optimize "on"