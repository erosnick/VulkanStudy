#include <stdio.h>
#include <stdint.h>
#include <thread>
#include <vector>
#include <filesystem>
#include <regex>
#include <thread>
#include <iostream>

#include <EtcLib/Etc/Etc.h>
#include <EtcLib/Etc/EtcImage.h>
#include <EtcLib/Etc/EtcFilter.h>
#include <EtcTool/EtcFile.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <optick.h>

std::vector<std::string> visit(std::string path)
{
	static std::vector<std::string> result;

	std::regex image(".*.*\\.(jpg|png|bmp)");	//包含字母z的所有jpg或png图片

	for (auto& fe : std::filesystem::directory_iterator(path))
	{
		auto fp = fe.path();

		if (!fp.has_extension())
		{
			visit(fp.string());
		}

		//std::wcout << fp.filename().wstring() << std::endl;

		auto temp = fp.filename();

		if (std::regex_match(temp.string(), image))
		{
			result.emplace_back(fp.string());
			//std::cout << temp << std::endl;
		}
		//replace_extension替换扩展名
		//stem去掉扩展名
	}

	return result;
}

void etc2Compress(const std::string& inputPath, const std::string& overrideOutputPath = "")
{
	int32_t width;
	int32_t height;
	int32_t comp;

	const uint8_t* imageData = stbi_load(inputPath.c_str(), &width, &height, &comp, 4);

	//float* imageData = stbi_loadf(inputPath.c_str(), &width, &height, &comp, 4);

	stbi_set_flip_vertically_on_load(true);
	stbi_ldr_to_hdr_scale(1.0f);

	std::vector<float> rgbaf;

	for (int i = 0; i != width * height * 4; i += 4)
	{
		rgbaf.push_back(imageData[i + 0] / 255.0f);
		rgbaf.push_back(imageData[i + 1] / 255.0f);
		rgbaf.push_back(imageData[i + 2] / 255.0f);
		rgbaf.push_back(imageData[i + 3] / 255.0f);
	}

	const auto etcFormat = Etc::Image::Format::RGB8;
	const auto errorMetric = Etc::ErrorMetric::BT709;

	Etc::Image image(rgbaf.data(), width, height, errorMetric);

	image.Encode(etcFormat, errorMetric, ETCCOMP_DEFAULT_EFFORT_LEVEL, std::thread::hardware_concurrency(), 1024);

	std::filesystem::path temp = inputPath;

	std::string outputPath = overrideOutputPath;

	if (outputPath.empty())
	{
		auto fileName = temp.replace_extension(".ktx").filename().string();
		auto directory = temp.remove_filename().string();

		outputPath = directory + "Compressed/" + fileName;
	}

	Etc::File etcFile(
		outputPath.c_str(),
		Etc::File::Format::KTX,
		etcFormat,
		image.GetEncodingBits(),
		image.GetEncodingBitsBytes(),
		image.GetSourceWidth(),
		image.GetSourceHeight(),
		image.GetExtendedWidth(),
		image.GetExtendedHeight()
	);
	etcFile.Write();

	std::cout << outputPath << std::endl;
}

int main()
{
	auto texturePathes = visit("../Assets/Textures");

	{
		OPTICK_PUSH("etc2Compress");
		etc2Compress("../Assets/Textures/ch2_sample3_STB.jpg");
		OPTICK_POP();
	}

	for (const auto& path : texturePathes)
	{
		etc2Compress(path);
	}

	return 0;
}
