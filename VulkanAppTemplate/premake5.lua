--workspace: 对应VS中的解决方案
workspace "VulkanApp"
    configurations { "Debug", "Release" }    --解决方案配置项，Debug和Release默认配置
    location "Project"                      --解决方案文件夹，这是我自己喜欢用的project简写
    cppdialect "c++17"    
    -- Turn on DPI awareness
    dpiawareness "High"

    --增加平台配置，我希望有Win32和x64两种平台
    platforms
    {
        "Win32",
        "x64"
    }

    --Win32平台配置属性
    filter "platforms:Win32"
        architecture "x86"      --指定架构为x86
        targetdir ("bin/%{cfg.buildcfg}_%{cfg.platform}/")      --指定输出目录
        objdir  ("obj/%{cfg.buildcfg}_%{cfg.platform}/")        --指定中间目录

    --x64平台配置属性
    filter "platforms:x64"
        architecture "x64"      --指定架构为x64
        targetdir ("bin/%{cfg.buildcfg}_%{cfg.platform}/")      --指定输出目录
        objdir  ("obj/%{cfg.buildcfg}_%{cfg.platform}/")        --指定中间目录

-- GLSLC helpers
dofile( "Utils/glslc.lua" )

project "Shaders"
	local shaders = { 
		"Assets/Shaders/*.vert",
		"Assets/Shaders/*.frag",
		"Assets/Shaders/*.comp",
		"Assets/Shaders/*.geom",
		"Assets/Shaders/*.tesc",
		"Assets/Shaders/*.tese"
	}

	kind "Utility"
	location "Assets/Shaders"

	files( shaders )

	handle_glsl_files( "-O", "Assets/Shaders", {} )

--project: 对应VS中的项目
project "VulkanApp"
    kind "ConsoleApp"                       --项目类型，控制台程序
    language "C++"                          --工程采用的语言，Premake5.0当前支持C、C++、C#
    dependson { "Shaders" }
    files 
    { 
        "%{prj.name}/src/**.h", 
        "%{prj.name}/src/**.cpp",
        "./ThirdParty/volk/src/volk.c",
        "./ThirdParty/tgen/src/tgen.cpp",
        "./ThirdParty/etc2comp/EtcTool/EtcFile.cpp",
        "./ThirdParty/etc2comp/EtcTool/EtcFileHeader.cpp",
        "./ThirdParty/VulkanMemoryAllocator/src/vk_mem_alloc.cpp"
    }                                       --指定加载哪些文件或哪些类型的文件

    vpaths 
    {
        -- ["Headers/*"] = { "*.h", "*.hpp" },  --包含具体路径
        -- ["Sources/*"] = { "*.c", "*.cpp" },  --包含具体路径
        ["Headers"] = { "**.h", "**.hpp" },     --不包含具体路径
        ["Sources"] = { "**.c", "**.cpp" },     --不包含具体路径
        ["Docs"] = "**.md"
    }

    excludes 
    { 
        "%{prj.name}/src/MeshBake/**.h", 
        "%{prj.name}/src/MeshBake/**.cpp", 
    }

    --Debug配置项属性
    filter "configurations:Debug"
        defines { "DEBUG", "FMT_HEADER_ONLY" }                 --定义Debug宏(这可以算是默认配置)
        symbols "On"                        --开启调试符号
        includedirs 
        { 
            './ThirdParty/tgen/include',
            './ThirdParty/stb',
            './ThirdParty/etc2comp',
            './ThirdParty/etc2comp/EtcLib/Etc',
            './ThirdParty/etc2comp/EtcLib/EtcCodec',
            '%{IncludeDir.VulkanSDK}',
            './ThirdParty/volk/include',
            './ThirdParty/imgui-1.89.2',
            './ThirdParty/tinyobjloader',
            './ThirdParty/glm-0.9.9.8/glm',
            './ThirdParty/fmt-9.1.0/include',
            './ThirdParty/Optick_1.4.0/include',
            './ThirdParty/rapidobj-1.0.1/include',
            './ThirdParty/glfw-3.3.8.bin.WIN64/include',
            './ThirdParty/VulkanMemoryAllocator/include',
            './ThirdParty/easy_profiler-v2.1.0-msvc15-win64/include',
        }

		libdirs 
        { 
            './ThirdParty/etc2comp/Lib',
            './ThirdParty/Optick_1.4.0/lib/x64/debug',
            './ThirdParty/glfw-3.3.8.bin.WIN64/lib-vc2022',
            './ThirdParty/easy_profiler-v2.1.0-msvc15-win64/lib'
        }

		links 
        { 
            "ImGui",
            "glfw3.lib", 
            "EtcLibd.lib",
            "OptickCore.lib",
            "%{Library.Vulkan}",
            "easy_profiler.lib"
        }

        debugdir "%{prj.location}"

    --Release配置项属性
    filter "configurations:Release"
        defines { "NDEBUG", "FMT_HEADER_ONLY" }                 --定义NDebug宏(这可以算是默认配置)
        optimize "On"                        --开启优化参数
        includedirs 
        { 
            './ThirdParty/tgen/include',
            './ThirdParty/stb',
            './ThirdParty/etc2comp',
            './ThirdParty/etc2comp/EtcLib/Etc',
            './ThirdParty/etc2comp/EtcLib/EtcCodec',
            '%{IncludeDir.VulkanSDK}',
            './ThirdParty/volk/include',
            './ThirdParty/imgui-1.89.2',
            './ThirdParty/tinyobjloader',
            './ThirdParty/glm-0.9.9.8/glm',
            './ThirdParty/fmt-9.1.0/include',
            './ThirdParty/Optick_1.4.0/include',
            './ThirdParty/rapidobj-1.0.1/include',
            './ThirdParty/glfw-3.3.8.bin.WIN64/include',
            './ThirdParty/VulkanMemoryAllocator/include',
            './ThirdParty/easy_profiler-v2.1.0-msvc15-win64/include'
        }

		libdirs 
        { 
            './ThirdParty/etc2comp/Lib',
            './ThirdParty/Optick_1.4.0/lib/x64/release',
            './ThirdParty/glfw-3.3.8.bin.WIN64/lib-vc2022',
            './ThirdParty/easy_profiler-v2.1.0-msvc15-win64/lib'
        }

		links 
        { 
            "ImGui",
            "glfw3.lib",
            "EtcLib.lib",
            "OptickCore.lib",
            "%{Library.Vulkan}", 
            "easy_profiler.lib" 
        }

        debugdir "%{prj.location}"

project "MeshBake"
    kind "ConsoleApp"                       --项目类型，控制台程序
    language "C++"                          --工程采用的语言，Premake5.0当前支持C、C++、C#
    location "Project"

    -- copy a file from the objects directory to the target directory
    postbuildcommands 
    {
    -- "{COPY} %{cfg.targetdir}/AriaCore.dll %{wks.location}"
    }

    files 
    { 
        "VulkanApp/src/MeshBake/**.h", 
        "VulkanApp/src/MeshBake/**.cpp", 
        "VulkanApp/src/labutils/error.hpp",
        "VulkanApp/src/labutils/error.cpp",
        "ThirdParty/tgen/src/tgen.cpp"
    }                                       --指定加载哪些文件或哪些类型的文件

    --Debug配置项属性
    filter "configurations:Debug"
        defines { "DEBUG", "ARIA_CORE_DEBUG", "ARIA_PLATFORM_WINDOWS" }                 --定义Debug宏(这可以算是默认配置)
        symbols "On"                                           --开启调试符号
        debugdir "%{prj.location}"

        includedirs 
        { 
            './ThirdParty/tgen/include',
            './ThirdParty/stb',
            './ThirdParty/etc2comp',
            './ThirdParty/etc2comp/EtcLib/Etc',
            './ThirdParty/etc2comp/EtcLib/EtcCodec',
            '%{IncludeDir.VulkanSDK}',
            './ThirdParty/volk/include',
            './ThirdParty/imgui-1.89.2',
            './ThirdParty/tinyobjloader',
            './ThirdParty/glm-0.9.9.8/glm',
            './ThirdParty/fmt-9.1.0/include',
            './ThirdParty/Optick_1.4.0/include',
            './ThirdParty/rapidobj-1.0.1/include',
            './ThirdParty/glfw-3.3.8.bin.WIN64/include',
            './ThirdParty/VulkanMemoryAllocator/include',
            './ThirdParty/easy_profiler-v2.1.0-msvc15-win64/include',
        }

		libdirs 
        { 
            './ThirdParty/etc2comp/Lib',
            './ThirdParty/Optick_1.4.0/lib/x64/release',
            './ThirdParty/glfw-3.3.8.bin.WIN64/lib-vc2022',
            './ThirdParty/easy_profiler-v2.1.0-msvc15-win64/lib'
        }

		links 
        { 
            "ImGui",
            "glfw3.lib", 
            "EtcLibd.lib",
            "OptickCore.lib",
            "%{Library.Vulkan}",
            "easy_profiler.lib"
        }

    --Release配置项属性
    filter "configurations:Release"
        defines { "NDEBUG", "ARIA_RELEASE", "ARIA_PLATFORM_WINDOWS" }                 --定义NDebug宏(这可以算是默认配置)
        optimize "On"                                           --开启优化参数
        debugdir "%{prj.location}"

        includedirs 
        { 
            './ThirdParty/tgen/include',
            './ThirdParty/stb',
            './ThirdParty/etc2comp',
            './ThirdParty/etc2comp/EtcLib/Etc',
            './ThirdParty/etc2comp/EtcLib/EtcCodec',
            '%{IncludeDir.VulkanSDK}',
            './ThirdParty/volk/include',
            './ThirdParty/imgui-1.89.2',
            './ThirdParty/tinyobjloader',
            './ThirdParty/glm-0.9.9.8/glm',
            './ThirdParty/fmt-9.1.0/include',
            './ThirdParty/Optick_1.4.0/include',
            './ThirdParty/rapidobj-1.0.1/include',
            './ThirdParty/glfw-3.3.8.bin.WIN64/include',
            './ThirdParty/VulkanMemoryAllocator/include',
            './ThirdParty/easy_profiler-v2.1.0-msvc15-win64/include',
        }

		libdirs 
        { 
            './ThirdParty/etc2comp/Lib',
            './ThirdParty/Optick_1.4.0/lib/x64/release',
            './ThirdParty/glfw-3.3.8.bin.WIN64/lib-vc2022',
            './ThirdParty/easy_profiler-v2.1.0-msvc15-win64/lib'
        }

		links 
        { 
            "ImGui",
            "glfw3.lib", 
            "EtcLibd.lib",
            "OptickCore.lib",
            "%{Library.Vulkan}",
            "easy_profiler.lib"
        }

include "Etc2Compress"
include "External.lua"