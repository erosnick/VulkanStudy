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
        "ThirdParty/volk/src/volk.c",
        "ThirdParty/VulkanMemoryAllocator/src/vk_mem_alloc.cpp"
    }                                       --指定加载哪些文件或哪些类型的文件

    vpaths 
    {
        -- ["Headers/*"] = { "*.h", "*.hpp" },  --包含具体路径
        -- ["Sources/*"] = { "*.c", "*.cpp" },  --包含具体路径
        ["Headers"] = { "**.h", "**.hpp" },     --不包含具体路径
        ["Sources"] = { "**.c", "**.cpp" },     --不包含具体路径
        ["Docs"] = "**.md"
    }

    --Debug配置项属性
    filter "configurations:Debug"
        defines { "DEBUG", "FMT_HEADER_ONLY" }                 --定义Debug宏(这可以算是默认配置)
        symbols "On"                        --开启调试符号
        includedirs 
        { 
            './ThirdParty/stb',
            '%{IncludeDir.VulkanSDK}',
            './ThirdParty/volk/include',
            './ThirdParty/imgui-1.89.2',
            './ThirdParty/tinyobjloader',
            './ThirdParty/glm-0.9.9.8/glm',
            './ThirdParty/fmt-9.1.0/include',
            './ThirdParty/rapidobj-1.0.1/include',
            './ThirdParty/glfw-3.3.8.bin.WIN64/include',
            './ThirdParty/VulkanMemoryAllocator/include'
        }

		libdirs 
        { 
            './ThirdParty/glfw-3.3.8.bin.WIN64/lib-vc2022'
        }

		links 
        { 
            "glfw3.lib", 
            "%{Library.Vulkan}", 
            "ImGui" 
        }

        debugdir "%{prj.location}"

    --Release配置项属性
    filter "configurations:Release"
        defines { "NDEBUG", "FMT_HEADER_ONLY" }                 --定义NDebug宏(这可以算是默认配置)
        optimize "On"                        --开启优化参数
        includedirs 
        { 
            './ThirdParty/stb',
            '%{IncludeDir.VulkanSDK}',
            './ThirdParty/volk/include',
            './ThirdParty/imgui-1.89.2',
            './ThirdParty/tinyobjloader',
            './ThirdParty/glm-0.9.9.8/glm',
            './ThirdParty/fmt-9.1.0/include',
            './ThirdParty/rapidobj-1.0.1/include',
            './ThirdParty/glfw-3.3.8.bin.WIN64/include',
            './ThirdParty/VulkanMemoryAllocator/include'
        }

		libdirs 
        { 
            './ThirdParty/glfw-3.3.8.bin.WIN64/lib-vc2022'
        }

		links 
        { 
            "glfw3.lib", 
            "%{Library.Vulkan}", 
            "ImGui" 
        }

        debugdir "%{prj.location}"

include "External.lua"