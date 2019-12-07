#pragma once
#include <string>
#include <fstream>
#include <VMat/geometry.h>
#include <vector>
#include "VMat/numeric.h"
#include <set>

struct BlockID
{
	int x, y, z;
	bool operator==(const BlockID& other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}

	bool operator<(const BlockID& other) const
	{
		return x < other.x ? true : (y == other.y ? z < other.z : y < other.y);
	}
	std::vector<int> toArray() const
	{
		return { x,y,z };
	}
};



class BlockAddressing
{
public:
	BlockAddressing();
	~BlockAddressing();
	void calcBlockArray(const std::string& file_name, const int block_based, const int padding, ysl::Vec3i volume_size, std::set<BlockID>& block_set);
	
	bool readObj(const std::string& filename, std::vector<ysl::Point3f>& point_set);

	std::vector<int> calcBlockMask(int block_based, const std::set<BlockID>& block_set,
	                                      std::vector<std::vector<std::vector<int>>>& block_mask_array);

	std::vector<ysl::Point3f>& getPointSet() { return point_set; }
	
private:
	std::vector<ysl::Point3f> point_set;
};

inline BlockAddressing::BlockAddressing()
= default;

inline BlockAddressing::~BlockAddressing()
= default;

inline void BlockAddressing::calcBlockArray(const std::string& file_name, const int block_based, const int padding,
	ysl::Vec3i volume_size, std::set<BlockID>& block_set)
{
	//open obj files	
	
	readObj(file_name, point_set);

	const ysl::Vec3i volumeDataSizeNoRepeat = volume_size;
	int b = (int)(std::pow(2, block_based) - 2 * padding);
	const ysl::Vec3i blockDataSizeNoRepeat = { b,b,b };
	const int px = ysl::RoundUpDivide(volume_size.x, b);
	const int py = ysl::RoundUpDivide(volume_size.y, b);
	const int pz = ysl::RoundUpDivide(volume_size.z, b);
	ysl::Vec3i pageTableSize = { px,py,pz };


	for (auto& point : point_set)
	{
		// address translation
		//const auto x = point.x * volumeDataSizeNoRepeat.x / (blockDataSizeNoRepeat.x * pageTableSize.x)
		const auto x = (point.x / volume_size.x * volumeDataSizeNoRepeat.x / (blockDataSizeNoRepeat.x * pageTableSize.x) * pageTableSize.x);
		const auto y = (point.y / volume_size.y * volumeDataSizeNoRepeat.y / (blockDataSizeNoRepeat.y * pageTableSize.y) * pageTableSize.y);
		const auto z = (point.z / volume_size.z * volumeDataSizeNoRepeat.z / (blockDataSizeNoRepeat.z * pageTableSize.z) * pageTableSize.z);
		//ivec3 entry3DIndex = ivec3(samplePos*pageTableSize);
		//unsigned int entryFlatIndex = entry3DIndex.z * pageTableSize.x*pageTableSize.y + entry3DIndex.y * pageTableSize.x + entry3DIndex.x;
		//auto blockIndex = ysl::Linear({ x,y,z }, {pageTableSize.x,pageTableSize.y});

		block_set.insert({ (int)x,(int)y,(int)z });
	}
	std::cout << "Block number for obj file : " << file_name << " has been calculated" << std::endl;

}

inline bool BlockAddressing::readObj(const std::string& filename, std::vector<ysl::Point3f>& point_set)
{
	std::ifstream in(filename.c_str());
	if (!in.good())
	{
		std::cout << "ERROR: loading obj:(" << filename << ") file is not good" << "\n";
		exit(0);
	}

	char buffer[256], str[255];
	float f1, f2, f3;
	while (!in.getline(buffer, 255).eof())
	{
		buffer[255] = '\0';

		sscanf_s(buffer, "%s", str, 255);

		// reading a vertex  
		if (buffer[0] == 'v' && (buffer[1] == ' ' || buffer[1] == 32))
		{
			if (sscanf(buffer, "v %f %f %f", &f1, &f2, &f3) == 3)
			{
				point_set.push_back({ f1, f2, f3 });
				// mesh.verts.push_back(float3(f1, f2, f3));
			}
			else
			{
				std::cout << "ERROR: vertex not in wanted format in OBJLoader" << "\n";
				exit(-1);
			}
		}
		else
		{

		}
	}
	return false;
}

inline std::vector<int> BlockAddressing::calcBlockMask(int block_based, const std::set<BlockID>& block_set,
                                                       std::vector<std::vector<std::vector<int>>>& block_mask_array)
{
	std::vector<int> block_max_bounding(3, -0xfffffff);
	std::vector<int> block_min_bounding(3, 0xfffffff);

	std::vector<std::vector<int>> block_array;
	for (auto& block : block_set)
	{
		block_array.push_back(block.toArray());
		block_max_bounding[0] = std::max(block_max_bounding[0], block.x);
		block_max_bounding[1] = std::max(block_max_bounding[1], block.y);
		block_max_bounding[2] = std::max(block_max_bounding[2], block.z);

		block_min_bounding[0] = std::min(block_min_bounding[0], block.x);
		block_min_bounding[1] = std::min(block_min_bounding[1], block.y);
		block_min_bounding[2] = std::min(block_min_bounding[2], block.z);
	}

	double size = block_set.size() * pow(2, block_based * 3) / 1024 / 1024;

	unsigned long long bounding_number = (block_max_bounding[0] - block_min_bounding[0] + 1) *
		(block_max_bounding[1] - block_min_bounding[1] + 1) *
		(block_max_bounding[2] - block_min_bounding[2] + 1);

	std::cout << "Block number :\t\t" << block_set.size() << std::endl;
	std::cout << "Bounding block number :\t" << bounding_number << std::endl;

	std::cout << "Size of the block data :\t" << size << "MB" << std::endl;

	size = (block_max_bounding[0] - block_min_bounding[0] + 1) *
		(block_max_bounding[1] - block_min_bounding[1] + 1) *
		(block_max_bounding[2] - block_min_bounding[2] + 1)
		* pow(2, block_based * 3) / 1024 / 1024 / 1024;

	std::cout << "Size of the bounding box data is : " << size << "GB" << std::endl;



	block_mask_array.resize(block_max_bounding[2] - block_min_bounding[2] + 1);
	for (auto& i : block_mask_array)
	{
		i.resize(block_max_bounding[1] - block_min_bounding[1] + 1);
		for (auto& j : i)
		{
			j.resize(block_max_bounding[0] - block_min_bounding[0] + 1,0);
		}
	}

	for(auto & block: block_set)
	{
		block_mask_array[block.z-block_min_bounding[2]][block.y - block_min_bounding[1]][block.x - block_min_bounding[0]] = 1;
	}
	return block_min_bounding;
}
