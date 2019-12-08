#include <iostream>
#include <VMFoundation/rawreader.h>
#include <fstream>
int main()
{
	ysl::RawReaderIO reader(R"(M:\original\mouse28452x21866x4834_lod0.raw)", { 28452,21866,4834 }, 1);

	unsigned char* data = new unsigned char[128 * 128 * 128];
	//reader.readRegion({ 5950,5950,2478 }, { 128,128,128 }, data);
	//reader.readRegion({ 5454,6446,2478 }, { 128,128,128 }, data);
	reader.readRegion({ 5950,6818,2478 }, { 128,128,128 }, data);

	std::ofstream writer("./subregion_test.raw", std::ios::binary);
	writer.write((const char *)data, 128 * 128 * 128);

	writer.close();
}