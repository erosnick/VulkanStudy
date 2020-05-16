#include "objLoader.h"
#include <cstdio>

bool objLoader::load(const std::string& fileName)
{
	char buffer[] = { "f 591/602/576 622/633/606 590/600/574" };

	unsigned int v1, v2, v3;
	unsigned int n1, n2, n3;
	unsigned int t1, t2, t3;

	sscanf_s(buffer, "f %d/%d/%d %d/%d/%d %d/%d/%d", &v1, &n1, &t1, &v2, &n2, &t2, &v3, &n3, &t3);

	return true;
}
