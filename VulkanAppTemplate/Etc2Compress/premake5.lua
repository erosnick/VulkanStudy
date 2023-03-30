project "Etc2Compress"
	kind "ConsoleApp"
	language "C++"

   files 
    { 
        "src/**.h", 
        "src/**.cpp",
        "../ThirdParty/etc2comp/EtcTool/EtcFile.cpp",
        "../ThirdParty/etc2comp/EtcTool/EtcFileHeader.cpp",
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
            '../ThirdParty/stb',
            '../ThirdParty/etc2comp',
            '../ThirdParty/etc2comp/EtcLib/Etc',
            '../ThirdParty/etc2comp/EtcLib/EtcCodec',
            '../ThirdParty/Optick_1.4.0/include',
        }

		libdirs 
        { 
            '../ThirdParty/etc2comp/Lib',
            '../ThirdParty/Optick_1.4.0/lib/x64/debug',
            '../ThirdParty/easy_profiler-v2.1.0-msvc15-win64/lib'
        }

		links 
        { 
            "EtcLibd.lib",
            "OptickCore.lib",
            "easy_profiler.lib"
        }

        debugdir "%{prj.location}"

    --Release配置项属性
    filter "configurations:Release"
        defines { "NDEBUG", "FMT_HEADER_ONLY" }                 --定义NDebug宏(这可以算是默认配置)
        optimize "On"                        --开启优化参数
              includedirs 
        { 
            '../ThirdParty/stb',
            '../ThirdParty/etc2comp',
            '../ThirdParty/etc2comp/EtcLib/Etc',
            '../ThirdParty/etc2comp/EtcLib/EtcCodec',
            '../ThirdParty/Optick_1.4.0/include',
        }

		libdirs 
        { 
            '../ThirdParty/etc2comp/Lib',
            '../ThirdParty/Optick_1.4.0/lib/x64/debug',
            '../ThirdParty/easy_profiler-v2.1.0-msvc15-win64/lib'
        }

		links 
        { 
            "EtcLib.lib",
            "OptickCore.lib",
            "easy_profiler.lib"
        }

        debugdir "%{prj.location}"