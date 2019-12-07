
#include <iostream>

#include <VMat/numeric.h>
#include <VMUtils/log.hpp>
#include <VMFoundation/pluginloader.h>
#include <VMCoreExtension/i3dblockfileplugininterface.h>

#include "BlockDataReader.h"
#include "json_struct.h"
#include "BlockAddressing.h"
#include "BlockDataWriter.h"
#include <stack>


bool isInBlock(const ysl::Size3& volume_size, int block_based, int padding, const ysl::Point3f& point, ysl::Vec3i block_id)
{
	const ysl::Vec3i volumeDataSizeNoRepeat = { (int)volume_size.x,(int)volume_size.y,(int)volume_size.z };
	int b = (int)(std::pow(2, block_based) - 2 * padding);
	const ysl::Vec3i blockDataSizeNoRepeat = { b,b,b };
	const int px = ysl::RoundUpDivide(volume_size.x, b);
	const int py = ysl::RoundUpDivide(volume_size.y, b);
	const int pz = ysl::RoundUpDivide(volume_size.z, b);
	ysl::Vec3i pageTableSize = { px,py,pz };

	//vm::println("page Table size : {}", pageTableSize);
	
	const int x = (point.x / volume_size.x * volumeDataSizeNoRepeat.x / (blockDataSizeNoRepeat.x * pageTableSize.x) * pageTableSize.x);
	const int y = (point.y / volume_size.y * volumeDataSizeNoRepeat.y / (blockDataSizeNoRepeat.y * pageTableSize.y) * pageTableSize.y);
	const int z = (point.z / volume_size.z * volumeDataSizeNoRepeat.z / (blockDataSizeNoRepeat.z * pageTableSize.z) * pageTableSize.z);

	//if(block_id.x == (int)x && block_id.y == (int)y && block_id.z == (int)z)
	//{
	//	vm::println("{},{}", ysl::Size3(x, y, z), block_id);
	//}
	//
	return block_id.x == x && block_id.y == y && block_id.z== z;
}

int main()
{
	

	std::string obj_file = R"(E:\14193_30neurons\N005.obj)";
	std::string lvd_file = R"(M:\mouselod0.lvd)";
	std::string blk_file = R"(./test.blk)";
	//struct InputJsonStruct ObjFilesJson;


	BlockDataReader reader;
	try
	{
		//reader.open(R"(D:\project\science_project\BlockRegionGrow\out\install\x64-Release\bin\test.blk)");
		reader.open(lvd_file);
	}
	catch (std::exception & e)
	{
		vm::println("{}", e.what());
	}
	
	int block_based = reader.getBlockBased();
	int padding = reader.getPadding();
	auto dimension = reader.getDataSizeDimension();

	unsigned char threshold = 30;

	try
	{
		/*std::ifstream input_stream(obj_path);
		input_stream >> ObjFilesJson;*/
		
		std::set<BlockID> block_set;
		BlockAddressing block_addressing;
		
		block_addressing.calcBlockArray(obj_file, block_based, padding, { (int)dimension.x,(int)dimension.y,(int)dimension.z }, block_set);

		std::vector<std::vector<std::vector<int>>> block_mask_array;
		auto block_min_bounding = block_addressing.calcBlockMask(block_based, block_set, block_mask_array);


		BlockDataWriter block_writer;

		block_writer.open(blk_file);
		
		auto block_size = reader.getBlockSize();
		auto volume_size = reader.getDataSizeDimension();
		ysl::Size3 block_number = {  block_mask_array[0][0].size(), block_mask_array[0].size(),block_mask_array.size() };

		auto block_size_without_padding = block_size - ysl::Size3(2 * padding, 2 * padding, 2 * padding);
		block_size_without_padding.x *= block_number.x;
		block_size_without_padding.y *= block_number.y;
		block_size_without_padding.z *= block_number.z;
		
		block_writer.writeHead(block_size_without_padding);			//GetDataSizeWithoutPadding	24B
		block_writer.writeHead(block_number);
																	//Get3DPageCount			24B
		block_writer.writeHead(block_size);							//Get3DPageSize				24B
		block_writer.writeHead(block_size.Prod());				//GetPageSize				8B
		block_writer.writeHead(block_number.Prod());			//GetPhysicalPageCount		8B
		block_writer.writeHead(block_number.Prod());			//GetVirtualPageCount		8B
		block_writer.writeHead(block_based);						//Get3DPageSizeInLog		8B
		block_writer.writeHead(padding);							//GetPadding				8B

		
		//The blk file has 336Byte head information

		//const int dx[6] = { -1,0,0,1,0,0 };
		//const int dy[6] = { 0,-1,0,0,1,0 };
		//const int dz[6] = { 0,0,-1,0,0,1 };

		const int offset[6] = { 1,-1, block_size.x , -block_size.x, block_size.x * block_size.y, -block_size.x * block_size.y };

		int cnt = 0;
		
		for (auto k = 0;k<block_mask_array.size();k++)
		{
			for (auto j = 0;j<block_mask_array[k].size();j++)
			{
				for (auto i=0;i<block_mask_array[k][j].size();i++)
				{
					const ysl::Vec3i block_id = { i + block_min_bounding[0], j + block_min_bounding[1], k + block_min_bounding[2] };
					//空串
					if(block_mask_array[k][j][i] == 0)
					{
						std::vector<unsigned char> block_data(block_size.Prod(), 0);
						block_writer.writeBody(block_data.data(), sizeof(unsigned char) * block_size.Prod());
					}
					//非空串
					else
					{
						cnt++;
						//Global block id
						const ysl::Vec3i block_id = { i + block_min_bounding[0], j + block_min_bounding[1], k + block_min_bounding[2] };

						vm::println("{}", block_id);
						
						//针对每个block，进行区域增长
						const unsigned char* block_data = 
							static_cast<const unsigned char*>(reader.getBlock(block_id.x, block_id.y, block_id.z));

						std::vector<unsigned char> block_vector(block_size.Prod(),0);

						for(const auto& point: block_addressing.getPointSet())
						{
							//针对在块内的点，作为种子点，进行区域增长。
							//TODO point 与块不对应
							if(isInBlock(volume_size,block_based, padding, point, block_id))
							{
								vm::println("{}", point);
								int index = ysl::Linear({ (int)((int)(point.x+0.5)-block_min_bounding[0]*block_size.x),
									(int)((int)(point.y+0.5) - block_min_bounding[1] * block_size.y),
									(int)((int)(point.z+0.5) - block_min_bounding[2] * block_size.z)},
									{ block_size.x, block_size.y });
								vm::println("Index : {}", index);

								if(index<0 || index >= block_size.Prod())
								{
									vm::println("Index out of range.");
									continue;
								}
								auto seed_value = block_data[index];

								vm::println("Seed value : {}", (int)seed_value);
								
								if(seed_value < threshold)
								{
									vm::println("Value is not avaliable.");
									continue;
								}
								std::stack<int> index_stack;

								index_stack.push(index);
								while(!index_stack.empty())
								{
									auto index = index_stack.top();
									index_stack.pop();
									block_vector[index] = block_data[index];

									for(auto idx = 0;idx < 6;idx++)
									{
										auto buf_index = index + offset[idx];
										if (buf_index<0 || buf_index>block_size.Prod() || block_vector[buf_index]) continue;
										if(block_vector[buf_index]==0 && block_data[buf_index]>=threshold)
										{
											index_stack.push(buf_index);
										}
									}
									
								}
							}
							
						}
						int num = 0;
						for (auto buf : block_vector) if (buf) num++;

						block_writer.writeBody(block_vector.data(), sizeof(unsigned char) * block_size.Prod());
						vm::println("Block {} has been written. It has {} non-empty voxels.", block_id, num);
					}
					
				}
			}
		}

		vm::println("Number of non-empty blocks : \t{}", cnt);
		
		
	}
	catch (std::exception & e)
	{
		vm::println("{}", e.what());
	}
	
	

	return 0;
}
