
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
#include "VMFoundation/rawreader.h"


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
	
	/*const int x = (point.x / volume_size.x * volumeDataSizeNoRepeat.x / (blockDataSizeNoRepeat.x * pageTableSize.x) * pageTableSize.x);
	const int y = (point.y / volume_size.y * volumeDataSizeNoRepeat.y / (blockDataSizeNoRepeat.y * pageTableSize.y) * pageTableSize.y);
	const int z = (point.z / volume_size.z * volumeDataSizeNoRepeat.z / (blockDataSizeNoRepeat.z * pageTableSize.z) * pageTableSize.z);*/
	const int x = point.x / blockDataSizeNoRepeat.x;
	const int y = point.y /blockDataSizeNoRepeat.y;
	const int z = point.z / blockDataSizeNoRepeat.z;

	//if(block_id.x == (int)x && block_id.y == (int)y && block_id.z == (int)z)
	//{
	//	vm::println("{},{}", ysl::Size3(x, y, z), block_id);
	//}
	//
	return block_id.x == x && block_id.y == y && block_id.z== z;
}



int transGlobalIndex(const ysl::Point3i& local_point, const ysl::Size3& block_size, const int padding)
{
	ysl::Point3i global_point = local_point;
	global_point.x += padding;
	global_point.y += padding;
	global_point.z += padding;
	return ysl::Linear({ global_point.x,global_point.y,global_point.z }, { block_size.x, block_size.y });
}

ysl::Point3i transGlobalPoint(const ysl::Point3i& local_point, const ysl::Size3& block_size, const int padding)
{
	ysl::Point3i global_point = local_point;
	global_point.x += padding;
	global_point.y += padding;
	global_point.z += padding;
	return global_point;
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

		//const ysl::Vec3i block_id = { 3 + 44, 1 + 48, 0 + 20 };
		//auto block_data = 
		//				(reader.getBlock(block_id.x, block_id.y, block_id.z));
		//std::ofstream test_writer("./block_test.raw", std::ios::binary);
		//test_writer.write((const char*)block_data, reader.getBlockSize().Prod());
		//test_writer.close();


		//ysl::RawReaderIO raw_reader(R"(M:\original\mouse28452x21866x4834_lod0.raw)", { 28452,21866,4834 }, 1);

		//unsigned char* buf_data = new unsigned char[128 * 128 * 128];
		//raw_reader.readRegion({ 5828,6074,2478 }, { 128,128,128 }, buf_data);
		//
		//test_writer.open("./raw_test.raw", std::ios::binary);
		//test_writer.write((const char*)buf_data, reader.getBlockSize().Prod());
		//test_writer.close();
		////ysl::RawReaderIO raw_reader(R"(M:\original\mouse28452x21866x4834_lod0.raw)", { 28452,21866,4834 }, 1);
		//return 0;
		
		
		//vm::println("{}", block_id);

		//针对每个block，进行区域增长
		//const unsigned char* block_data = 
		//	static_cast<const unsigned char*>(reader.getBlock(block_id.x, block_id.y, block_id.z));


		
		//Debug
		//char * data = (char *)reader.getBlock(1628);

		//std::ofstream test_writer("./getpage_test.raw", std::ios::binary);
		//test_writer.write(data, reader.getBlockSize().Prod());
		//test_writer.close();
		//return 0;
		
	}
	catch (std::exception & e)
	{
		vm::println("{}", e.what());
	}
	
	int block_based = reader.getBlockBased();
	int padding = reader.getPadding();
	auto dimension = reader.getDataSizeDimension();

	unsigned char threshold = 20;

	const double max_distance = std::sqrt(std::pow(8,3));

	try
	{
		/*std::ifstream input_stream(obj_path);
		input_stream >> ObjFilesJson;*/
		
		std::set<BlockID> block_set;
		BlockAddressing block_addressing;
		
		block_addressing.calcBlockArray(obj_file, block_based, padding, { (int)dimension.x,(int)dimension.y,(int)dimension.z }, block_set);

		std::vector<std::vector<std::vector<int>>> block_mask_array;		//z y x
		auto block_min_bounding = block_addressing.calcBlockMask(block_based, block_set, block_mask_array);


		BlockDataWriter block_writer;

		block_writer.open(blk_file);
		
		auto block_size = reader.getBlockSize();
		auto volume_size = reader.getDataSizeDimension();
		ysl::Size3 block_number = {  block_mask_array[0][0].size(), block_mask_array[0].size(),block_mask_array.size() };	//x y z

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


		//存储非空块的index
		
		//The blk file has 336Byte head information

		//const int dx[6] = { -1,0,0,1,0,0 };
		//const int dy[6] = { 0,-1,0,0,1,0 };
		//const int dz[6] = { 0,0,-1,0,0,1 };

		const int offset[6] = { 1,-1, block_size.x , -block_size.x, block_size.x * block_size.y, -block_size.x * block_size.y };

		int cnt = 0;

		//ysl::RawReaderIO raw_reader(R"(M:\original\mouse28452x21866x4834_lod0.raw)", { 28452,21866,4834 }, 1);
		
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
						
						//Global block id
						const ysl::Vec3i block_id = { i + block_min_bounding[0], j + block_min_bounding[1], k + block_min_bounding[2] };

						//vm::println("{}", block_id);
						
						//针对每个block，进行区域增长
						const unsigned char* block_data = 
							static_cast<const unsigned char*>(reader.getBlock(block_id.x, block_id.y, block_id.z));

						
						ysl::Point3i start_point = ysl::Point3i(block_id.x * (block_size.x - 2 * padding) - padding,
							block_id.y * (block_size.y - 2 * padding) - padding,
							block_id.z * (block_size.z - 2 * padding) - padding);

						vm::println("Block {} : start point : {} end before point : {}", block_id, start_point,
							ysl::Point3i(block_id.x * (block_size.x - 2 * padding) + block_size.x - padding,
								block_id.y * (block_size.y - 2 * padding) + block_size.y - padding,
								block_id.z * (block_size.z - 2 * padding) + block_size.z - padding));


						//unsigned char* block_data = new unsigned char[block_size.Prod()];
						//reader.readRegion({ 5950,5950,2478 }, { 128,128,128 }, data);
						//reader.readRegion({ 5454,6446,2478 }, { 128,128,128 }, data);
						//raw_reader.readRegion({ start_point.x,start_point.y,start_point.z}, { block_size.x,block_size.y,block_size.z }, block_data);
						
						
						//if (start_point.x == 5950 && start_point.y == 6818 && start_point.z == 2478)
						{
							std::ofstream writer("./subregion_6694_10662_2602_128_128_128.raw", std::ios::binary);
							writer.write((const char*)block_data, block_size.Prod());
							writer.close();
							//return 0;
						}

						
						std::vector<unsigned char> block_vector(block_size.Prod(), 0);
						std::vector<int> block_mask_vector(block_size.Prod(),0);

						for(const auto& point: block_addressing.getPointSet())
						{
							//针对在块内的点，作为种子点，进行区域增长。
							//TODO point 与块不对应
							if(isInBlock(volume_size,block_based, padding, point, block_id))
							{
								

								ysl::Point3i local_point = { (int)((int)(point.x + 0.5) - block_id.x * (block_size.x - 2 * padding)),
									(int)((int)(point.y + 0.5) - block_id.y * (block_size.y - 2 * padding)),
									(int)((int)(point.z + 0.5) - block_id.z * (block_size.z - 2 * padding)) };
								
								//int index = ysl::Linear(local_point,ysl::Size2( block_size.x - 2 * padding, block_size.y - 2 * padding ));

								auto global_point = transGlobalPoint(local_point, block_size, padding);
								
								int global_index = ysl::Linear({ global_point.x,global_point.y,global_point.z }, { block_size.x, block_size.y });

								int index = global_index;
								
								//vm::println("Point {} is in block {}, local point : {}, global point : {}", point, block_id, local_point, global_point);

								if(global_point.x<padding|| global_point.x >=block_size.x-padding ||
									global_point.y < padding || global_point.y >= block_size.y - padding || 
									global_point.z < padding || global_point.z >= block_size.z - padding)
								{
									vm::println("Index out of range.");
									continue;
								}

								
								auto seed_value = block_data[index];

								//vm::println("Seed value : {}", (int)seed_value);
								
								if(seed_value < threshold)
								{
									//vm::println("Value is not avaliable.");
									continue;
								}
								std::stack<int> index_stack;

								index_stack.push(index);
								while(!index_stack.empty())
								{
									index = index_stack.top();
									index_stack.pop();
									block_vector[index] = block_data[index];
									block_mask_vector[index] = 1;
									//vm::println("test3");

									for (auto idx = 0; idx < 6; idx++)
									{
										auto buf_index = index + offset[idx];

										int buf_x = buf_index % block_size.x;
										int buf_z = buf_index / (block_size.x * block_size.y);
										int buf_y = (buf_index % (block_size.x * block_size.y)) / block_size.x;

										ysl::Point3i buf_point = { buf_x,buf_y, buf_z };
										//if(buf_point.x < padding || buf_point.x >= buf_point.x - padding ||
										//	buf_point.y < padding || buf_point.y >= buf_point.y - padding ||
										//	buf_point.z < padding || buf_point.z >= buf_point.z - padding)
										//{
										//	//设定padding部分数据为原始数据
										//	block_vector[buf_index] = block_data[buf_index];
										//	continue;
										//}
										//
										//边界数据全部加上
										auto distance = buf_point - global_point;

										//if (distance.Length() > max_distance) continue;
										
										if (buf_index<0 || buf_index>=block_size.Prod() || block_mask_vector[buf_index]) continue;

										if (block_mask_vector[buf_index] == 0)
										{
											if (buf_point.x < padding || buf_point.x >= block_size.x - padding ||
												buf_point.y < padding || buf_point.y >= block_size.y - padding ||
												buf_point.z < padding || buf_point.z >= block_size.z - padding)
											{
												//设定padding部分数据为原始数据
												//index_stack.push(buf_index);
												//block_mask_vector[buf_index] = 1;
											}
											else if (block_data[buf_index] >= threshold)
											{
												index_stack.push(buf_index);
												block_mask_vector[buf_index] = 1;
											}
										}
										//if(buf_point.x < padding || buf_point.x >= buf_point.x - padding ||
										//	buf_point.y < padding || buf_point.y >= buf_point.y - padding ||
										//	buf_point.z < padding || buf_point.z >= buf_point.z - padding)
										//{
										//	//设定padding部分数据为原始数据
										//	index_stack.push(buf_index);
										//}
										//
										//else if(block_vector[buf_index]==-1 && block_data[buf_index]>=threshold)
										//{
										//	index_stack.push(buf_index);
										//}
									}
									


									//vm::println("test0");
								}
							}
						}
						int num = 0;
						for (auto buf : block_vector) if (buf) num++;


						block_writer.writeBody(block_vector.data(), sizeof(unsigned char) * block_size.Prod());
						//6322, 8306, 2602
						//if (start_point.x == 6322 && start_point.y == 8306 && start_point.z == 2602)
						{
							std::ofstream writer("./subregion_6694_10662_2602_128_128_128_region_growth.raw", std::ios::binary);
							writer.write((const char*)block_vector.data(), block_size.Prod());
							writer.close();
							//return 0;
						}
						
						vm::println("Block {} has been written. It has {} non-empty voxels.", block_id, num);
						//num++;

						if (num) cnt++;
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
