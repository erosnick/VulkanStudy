#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define BITMAP_MAGIC_NUMBER 19778


typedef struct bmp_file_header_s bmp_file_header_t;
struct bmp_file_header_s
{
	/*	int16_t magic_number; */ /* because of padding, we don't put it into the struct */
	int32_t size;
	int32_t app_id;
	int32_t offset;
};

typedef struct bmp_bitmap_info_header_s bmp_bitmap_info_header_t;
struct bmp_bitmap_info_header_s
{
	int32_t header_size;
	int32_t width;
	int32_t height;
	int16_t num_planes;
	int16_t bpp;
	int32_t compression;
	int32_t image_size;
	int32_t horizontal_resolution;
	int32_t vertical_resolution;
	int32_t colors_used;
	int32_t colors_important;
};

typedef struct bmp_palette_element_s bmp_palette_element_t;
struct bmp_palette_element_s
{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	unsigned char reserved;	/* alpha ? */
};

void flipVertical(unsigned char* buffer, int width, int height)
{
	unsigned char temp;
	for (int k = 0; k < (int)height >> 1; ++k) {
		unsigned char* p1 = buffer + k * width * 4;
		unsigned char* p2 = buffer + (height - 1 - k) * width * 4;
		for (int i = 0; i < (int)width * 4; ++i) {
			temp = p1[i]; p1[i] = p2[i]; p2[i] = temp;
		}
	}
}

static void bmpReadPixels32(FILE* file, unsigned char* dest, int width, int heigth)
{
	int i, j;
	unsigned char px[4], * p;

	p = dest;
	for (i = 0; i < heigth; i++) {
		for (j = 0; j < width; j++) {
			fread(px, 4, 1, file);
			*p = px[2]; p++; /* convert BGRX to RGBA */
			*p = px[1]; p++;
			*p = px[0]; p++;
			*p = 0xFF; p++;
		}
	}

	if (width > 0)
	{
		flipVertical(dest, width, heigth);
	}
}

static void bmpReadPixels24(FILE* file, unsigned char* dest, int width, int heigth)
{
	int i, j;
	unsigned char px[3], * p;

	p = dest;
	for (i = 0; i < heigth; i++) {
		for (j = 0; j < width; j++) {
			fread(px, 3, 1, file);
			*p = px[2]; p++;	/* convert BGR to RGBA */
			*p = px[1]; p++;
			*p = px[0]; p++;
			*p = 0xFF;  p++;	/* add alpha component */
		}
		if (width * 3 % 4 != 0)
			fseek(file, 4 - (width * 3 % 4), SEEK_CUR); /* if the width is not a multiple of 4, skip the end of the line */
	}

	if (width > 0)
	{
		flipVertical(dest, width, heigth);
	}
}

unsigned char* loadBMP(const char* filename, int* width, int* height)
{
	FILE* file;
	int16_t magic_number;
	bmp_file_header_t file_header;
	bmp_bitmap_info_header_t info_header;
	bmp_palette_element_t* palette = NULL;
	unsigned char* buf;

	fopen_s(&file, filename, "rb");
	if (!file) {
		printf("Error : could not open file %s\n", filename);
		return nullptr;
	}

	fread(&magic_number, 2, 1, file);
	if (magic_number != BITMAP_MAGIC_NUMBER) {
		printf("%s is not a valid bmp file : magic number is %d whereas it should be %d\n", filename, magic_number, BITMAP_MAGIC_NUMBER);
		fclose(file);
		return nullptr;
	}

	/* read headers */
	fread((void*)&file_header, 12, 1, file);
	fread((void*)&info_header, 40, 1, file);

	/* info_header sanity checks */
	/* accepted headers : bitmapinfoheader, bitmapv4header, bitmapv5header */
	if (!(info_header.header_size == 40 || info_header.header_size == 108 || info_header.header_size == 124)) {
		printf("Error : could not load %s : unknown info header\n", filename);
		printf("Info header size : %d\n", info_header.header_size);
		fclose(file);
		return nullptr;
	}
	if (info_header.num_planes != 1) {
		printf("%s can't be loaded. Number of planes is %d. ", filename, info_header.num_planes);
		printf("bmp files with more than one plane cannot be loaded\n");
		fclose(file);
		return nullptr;
	}
	if (info_header.compression == 4 || info_header.compression == 5) {
		printf("Error : could not load file %s : jpeg and png compressions are not supported\n", filename);
		fclose(file);
		return nullptr;
	}
	if (info_header.height < 0) {
		printf("Error : could not load file %s : top-down bitmaps are not supported\n", filename);
		fclose(file);
		return nullptr;
	}

	/* memory allocation */
	buf = (unsigned char*)malloc(info_header.width * info_header.height * 4);
	if (!buf) {
		printf("malloc() failed in function loadBMP()\n");
		if (info_header.colors_used) free(palette);
		fclose(file);
		return nullptr;
	}

	memset(buf, 0x00, info_header.width * info_header.height * 4);

	/* load image data */
	fseek(file, file_header.offset, SEEK_SET);
	switch (info_header.bpp)
	{
	case 32:
		bmpReadPixels32(file, buf, info_header.width, info_header.height);
		break;
	case 24:
		bmpReadPixels24(file, buf, info_header.width, info_header.height);
		break;
	}

	/* picture is ready, export data */
	*width = info_header.width;
	*height = info_header.height;

	if (info_header.colors_used) free(palette);
	fclose(file);

	return buf;
}
