#include "BakedModel.h"

#include <cstdio>
#include <cstring>

#include "labutils/error.hpp"

namespace lut = labutils;

namespace
{
	// See cw2-bake/main.cpp for more info
	constexpr char kFileMagic[16] = "\0\0COMP5822Mmesh";
	constexpr char kFileVariant[16] = "default-cw3";

	constexpr std::uint32_t kMaxString = 32*1024;

	// functions
	BakedModel loadBakedModel(FILE* inputFile, const std::string& modelPath);
}

BakedModel loadBakedModel(const std::string& modelPath)
{
	FILE* fin = std::fopen(modelPath.c_str(), "rb" );
	if(!fin)
		throw lut::Error( "load_baked_model(): unable to open '%s' for reading", modelPath);

	try
	{
		auto ret = loadBakedModel(fin, modelPath);
		std::fclose(fin);
		return ret;
	}
	catch( ... )
	{
		std::fclose(fin);
		throw;
	}
}

namespace
{
	void checked_read_( FILE* aFin, std::size_t aBytes, void* aBuffer )
	{
		auto ret = std::fread( aBuffer, 1, aBytes, aFin );

		if( aBytes != ret )
			throw lut::Error( "checked_read_(): expected %zu bytes, got %zu", aBytes, ret );
	}

	std::uint32_t read_uint32_( FILE* aFin )
	{
		std::uint32_t ret;
		checked_read_( aFin, sizeof(std::uint32_t), &ret );
		return ret;
	}
	std::string read_string_( FILE* aFin )
	{
		auto const length = read_uint32_( aFin );

		if( length >= kMaxString )
			throw lut::Error( "read_string_(): unexpectedly long string (%u bytes)", length );

		std::string ret;
		ret.resize( length );

		checked_read_( aFin, length, ret.data() );
		return ret;
	}

	BakedModel loadBakedModel(FILE* inputFile, const std::string& modelPath)
	{
		BakedModel ret;

		// Figure out base path
		char const* pathBeg = modelPath.c_str();
		char const* pathEnd = std::strrchr( pathBeg, '/' );
	
		std::string const prefix = pathEnd
			? std::string( pathBeg, pathEnd+1 )
			: ""
		;

		// Read header and verify file magic and variant
		char magic[16];
		checked_read_(inputFile, 16, magic);

		if( 0 != std::memcmp(magic, kFileMagic, 16))
			throw lut::Error("load_baked_model_(): %s: invalid file signature!", modelPath);

		char variant[16];
		checked_read_(inputFile, 16, variant);

		if( 0 != std::memcmp( variant, kFileVariant, 16 ) )
			throw lut::Error( "load_baked_model_(): %s: file variant is '%s', expected '%s'", modelPath, variant, kFileVariant );

		// Read texture info
		auto const textureCount = read_uint32_(inputFile);
		for( std::uint32_t i = 0; i < textureCount; ++i )
		{
			BakedTextureInfo info;
			info.path = prefix + read_string_(inputFile);

			std::uint8_t channels;
			checked_read_(inputFile, sizeof(std::uint8_t), &channels );
			info.channels = channels;

			ret.textures.emplace_back( std::move(info) );
		}

		// Read material info
		auto const materialCount = read_uint32_(inputFile);
		for( std::uint32_t i = 0; i < materialCount; ++i )
		{
			BakedMaterialInfo info;
			info.baseColorTextureId = read_uint32_(inputFile);
			info.roughnessTextureId = read_uint32_(inputFile);
			info.metalnessTextureId = read_uint32_(inputFile);
			info.alphaMaskTextureId = read_uint32_(inputFile);
			info.normalMapTextureId = read_uint32_(inputFile);
			
			assert( info.baseColorTextureId < ret.textures.size() );
			assert( info.roughnessTextureId < ret.textures.size() );
			assert( info.metalnessTextureId < ret.textures.size() );

			// New for Coursework 3:
			checked_read_(inputFile, sizeof(float)*3, &info.baseColor.x );
			checked_read_(inputFile, sizeof(float)*3, &info.emissiveColor.x );
			checked_read_(inputFile, sizeof(float), &info.roughness );
			checked_read_(inputFile, sizeof(float), &info.metalness );

			ret.materials.emplace_back( std::move(info) );
		}

		// Read mesh data
		auto const meshCount = read_uint32_(inputFile);
		for( std::uint32_t i = 0; i < meshCount; ++i )
		{
			BakedMeshData data;
			data.materialId = read_uint32_(inputFile);
			assert( data.materialId < ret.materials.size() );

			auto const V = read_uint32_(inputFile);
			auto const I = read_uint32_(inputFile);

			data.positions.resize( V );
			checked_read_(inputFile, V*sizeof(glm::vec3), data.positions.data());

			data.normals.resize( V );
			checked_read_(inputFile, V*sizeof(glm::vec3), data.normals.data());

			data.texcoords.resize( V );
			checked_read_(inputFile, V*sizeof(glm::vec2), data.texcoords.data());

			data.indices.resize( I );
			checked_read_(inputFile, I*sizeof(std::uint32_t), data.indices.data());

			ret.meshes.emplace_back( std::move(data));
		}

		// Check
		char byte;
		auto const check = std::fread( &byte, 1, 1, inputFile);
		
		if( 0 != check )
			std::fprintf(stderr, "Note: '%s' contains trailing bytes\n", modelPath.c_str());

		return ret;
	}
}
