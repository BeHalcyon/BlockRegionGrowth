
#include <iostream>

#include <VMat/numeric.h>
#include <VMUtils/log.hpp>
#include <VMFoundation/pluginloader.h>
#include <VMCoreExtension/i3dblockfileplugininterface.h>


#include <stack>
#include "VMFoundation/rawreader.h"
#include <VMUtils/cmdline.hpp>
#include <VMUtils/json_binding.hpp>
#include "../src/BlockAddressing.h"
#include "../src/BlockDataWriter.h"

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
	const int y = point.y / blockDataSizeNoRepeat.y;
	const int z = point.z / blockDataSizeNoRepeat.z;




	return block_id.x == x && block_id.y == y && block_id.z == z;
}

size_t readRegion(const ysl::Vec3i& start, const ysl::Size3& size, const ysl::Size3 dimensions, unsigned char* region_data, unsigned char* buffer)
{
	assert(size.x > 0 && size.y > 0 && size.z > 0);

	//TODO 有问题
	
	for (int k = 0; k <  size.z; k++)
	{
		for (int j = 0; j <  size.y; j++)
		{
			for (int i = 0; i < size.x; i++)
			{	
				auto local_id = ysl::Linear(ysl::Point3i(i , j, k ), ysl::Size2( size.x, size.y ));
				auto global_id = ysl::Linear(ysl::Point3i(i + start.x, j + start.y, k + start.z), ysl::Size2( dimensions.x,dimensions.y ));

				if (i + start.x < 0 || i + start.x >= dimensions.x || j + start.y < 0 ||
					j + start.y >= dimensions.y || k + start.z < 0 || k + start.z >= dimensions.z)
				{
					buffer[local_id] = 0;
				}
				else
					buffer[local_id] = region_data[global_id];
				
			}
		}
	}
	return size.Prod();
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		vm::println("Please use command line parameter.");
	}

	cmdline::parser a;
	a.add<std::string>("obj_files", 'o', "Input obj json file", false, R"(E:\14193_30neurons\obj_files.json)");
	a.parse_check(argc, argv);

	std::string obj_files = a.get<std::string>("obj_files");


	std::string obj_file = R"(E:\14193_30neurons\N005.obj)";
	

	int block_based = 7;
	int padding = 2;
	ysl::Size3 volume_size = { 28452,21866,4834 };

	ysl::RawReaderIO reader(R"(M:\original\mouse28452x21866x4834_lod0.raw)", volume_size, 1);



	try
	{
		/*std::ifstream input_stream(obj_path);
		input_stream >> ObjFilesJson;*/

		std::set<BlockID> block_set;
		BlockAddressing block_addressing;

		block_addressing.calcBlockArray(obj_file, block_based, padding, { (int)volume_size.x,(int)volume_size.y,(int)volume_size.z }, block_set);

		int b = (int)(std::pow(2, block_based));
		
		ysl::Size3 block_size = ysl::Size3(b , b, b );
		ysl::Size3 block_size_no_repeat = ysl::Size3(b - 2 * padding, b - 2 * padding, b - 2 * padding );
		
		std::vector<std::vector<std::vector<int>>> block_mask_array;		//z y x
		auto block_min_bounding = block_addressing.calcBlockMask(block_based, block_set, block_mask_array); //x y z

		ysl::Vec3i block_number = ysl::Vec3i(block_mask_array[0][0].size(),block_mask_array[0].size(), block_mask_array.size());
		
		ysl::Vec3i start_point = ysl::Vec3i(block_min_bounding[0] * block_size_no_repeat.x,
									block_min_bounding[1] * block_size_no_repeat.y,
									block_min_bounding[2] * block_size_no_repeat.z);

		ysl::Size3 region_dimension = { block_number.x * block_size_no_repeat.x,
										block_number.y * block_size_no_repeat.y,
										block_number.z * block_size_no_repeat.z };

		vm::println("Region dimension : {}", region_dimension);

		unsigned char* region_data = new unsigned char[region_dimension.Prod()];
		reader.readRegion(start_point, region_dimension, region_data);


		std::string blk_file = obj_file.substr(0, obj_file.find_last_of('.')) + ".blk";
		BlockDataWriter block_writer;
		block_writer.open(blk_file);

		block_writer.writeHead(region_dimension);					//GetDataSizeWithoutPadding	24B
		block_writer.writeHead(ysl::Size3(block_number.x, block_number.y, block_number.z));						//Get3DPageCount			24B
		block_writer.writeHead(block_size);							//Get3DPageSize				24B
		block_writer.writeHead(block_size.Prod());				//GetPageSize				8B
		block_writer.writeHead(block_number.Prod());			//GetPhysicalPageCount		8B
		block_writer.writeHead(block_number.Prod());			//GetVirtualPageCount		8B
		block_writer.writeHead(block_based);						//Get3DPageSizeInLog		8B
		block_writer.writeHead(padding);							//GetPadding				8B
		
		block_writer.writeHead(block_set.size());
		for (auto k = 0; k < block_mask_array.size(); k++)
		{
			for (auto j = 0; j < block_mask_array[k].size(); j++)
			{
				for (auto i = 0; i < block_mask_array[k][j].size(); i++)
				{
					//非空块
					if (block_mask_array[k][j][i] != 0)
					{
						auto block_id_1D = ysl::Linear({ i,j, k }, ysl::Size2( block_number.x,block_number.y ));
						block_writer.writeHead(block_id_1D);
					}
				}
			}
		}

		
		std::vector<bool> region_mask_array(region_dimension.Prod(), false);

		unsigned char threshold = 20;

		const size_t offset[6] = { 1,-1,
			region_dimension.x , -region_dimension.x,
			region_dimension.x * region_dimension.y, -region_dimension.x * region_dimension.y };

		const double max_distance = std::sqrt(std::pow(8, 3));
		
		for (const auto& point : block_addressing.getPointSet())
		{
			ysl::Point3i local_point = ysl::Point3i(point.x - start_point.x,point.y - start_point.y, point.z - start_point.z );
			if (local_point.x < 0 || local_point.y < 0 || local_point.z < 0 ||
				local_point.x >= region_dimension.x || local_point.y >= region_dimension.y || local_point.z >= region_dimension.z)
			{
				vm::println("Local point {} is not available.", local_point);
				continue;
			}

			//local_point = ysl::Point3i(point.x, point.y, point.z);

			auto index = ysl::Linear({ local_point.x, local_point.y, local_point.z }, { region_dimension.x, region_dimension.y });

			auto seed_value = region_data[index];

			if (seed_value < threshold)
			{
				continue;
			}
			std::stack<size_t> index_stack;

			index_stack.push(index);
			while (!index_stack.empty())
			{
				index = index_stack.top();
				index_stack.pop();
				region_mask_array[index] = true;

				for (auto idx = 0; idx < 6; idx++)
				{
					auto buf_index = index + offset[idx];

					auto buf_x = buf_index % region_dimension.x;
					auto buf_z = buf_index / (region_dimension.x * region_dimension.y);
					auto buf_y = (buf_index % (region_dimension.x * region_dimension.y)) / region_dimension.x;

					ysl::Point3i buf_point = ysl::Point3i( buf_x, buf_y, buf_z );

					//边界数据全部加上
					auto distance = buf_point - local_point;

					if (distance.Length() > max_distance) continue;

					if (buf_index < 0 || buf_index >= region_dimension.Prod() || region_mask_array[buf_index]) continue;

					if (!region_mask_array[buf_index])
					{

						if (region_data[buf_index] >= threshold)
						{
							index_stack.push(buf_index);
							region_mask_array[buf_index] = true;
						}
					}
				}
			}

		}
		size_t non_empty_count = 0;
		for (size_t i = 0; i < region_dimension.Prod(); i++)
			if (!region_mask_array[i])
			{
				region_data[i] = 0;
			}
			else non_empty_count++;
				
		vm::println("Non-empty voxels number is {}.", non_empty_count);

		const int offset1[6] = { 1,-1, block_size.x , -block_size.x, block_size.x * block_size.y, -block_size.x * block_size.y };

		
		for (auto k = 0; k < block_mask_array.size(); k++)
		{
			for (auto j = 0; j < block_mask_array[k].size(); j++)
			{
				for (auto i = 0; i < block_mask_array[k][j].size(); i++)
				{
					//非空块
					if (block_mask_array[k][j][i] != 0)
					{
						auto block_id_1D = ysl::Linear({ i,j, k }, ysl::Size2( block_number.x,block_number.y));

						ysl::Vec3i local_point = ysl::Vec3i((i) * block_size_no_repeat.x,
							(j) * block_size_no_repeat.y,
							(k) * block_size_no_repeat.z);
						
						ysl::Vec3i global_point = {local_point.x-padding, local_point.y - padding,local_point.z - padding };

						unsigned char* block_data = new unsigned char[block_size.Prod()];
						
						//索引对应块的真实数据
						//reader.readRegion(global_point, block_size, block_data);

						readRegion(global_point, block_size, region_dimension, region_data, block_data);


						int num = 0;
						for (auto p = 0; p < block_size.Prod(); p++) if (block_data[p]) num++;

						vm::println("Block {} has been written. Non-empty voxels number is {}",
							ysl::Size3(i, (j), k), num);
						
						if(num<1000)
						{

							ysl::Vec3i block_id = { i + block_min_bounding[0], j + block_min_bounding[1], k + block_min_bounding[2] };
							global_point = ysl::Vec3i(block_size_no_repeat.x * block_id.x - padding,
								block_size_no_repeat.y * block_id.y -padding,
								block_size_no_repeat.z * block_id.z-padding);
							
							reader.readRegion(global_point, block_size, block_data);
							std::vector<unsigned char> block_vector(block_size.Prod(), 0);
							std::vector<int> block_mask_vector(block_size.Prod(), 0);
							
							for(const auto & point : block_addressing.getPointSet())
							{
								//ysl::Vec3i block_id = { i + block_min_bounding[0], j + block_min_bounding[1], k + block_min_bounding[2] };
								if(isInBlock(volume_size, block_based, padding, point, block_id))
								{

									ysl::Point3i local_point = { (int)((int)(point.x + 0.5) - block_id.x * (block_size.x - 2 * padding)),
									(int)((int)(point.y + 0.5) - block_id.y * (block_size.y - 2 * padding)),
									(int)((int)(point.z + 0.5) - block_id.z * (block_size.z - 2 * padding)) };

									//int index = ysl::Linear(local_point,ysl::Size2( block_size.x - 2 * padding, block_size.y - 2 * padding ));

									ysl::Point3i global_point = { local_point.x + padding,local_point.y + padding, local_point.z + padding };

									int global_index = ysl::Linear({ global_point.x,global_point.y,global_point.z }, { block_size.x, block_size.y });

									int index = global_index;
									

									if (global_point.x < 0 || global_point.x >= block_size.x ||
										global_point.y < 0 || global_point.y >= block_size.y ||
										global_point.z < 0 || global_point.z >= block_size.z)
									{
										//vm::println("Index out of range.");
										continue;
									}


									auto seed_value = block_data[index];

									//vm::println("Seed value : {}", (int)seed_value);

									if (seed_value < threshold-5)
									{
										//vm::println("Value is not avaliable.");
										continue;
									}
									std::stack<int> index_stack;

									index_stack.push(index);
									while (!index_stack.empty())
									{
										index = index_stack.top();
										index_stack.pop();
										block_vector[index] = block_data[index];
										block_mask_vector[index] = 1;
										//vm::println("test3");

										for (auto idx = 0; idx < 6; idx++)
										{
											auto buf_index = index + offset1[idx];

											int buf_x = buf_index % block_size.x;
											int buf_z = buf_index / (block_size.x * block_size.y);
											int buf_y = (buf_index % (block_size.x * block_size.y)) / block_size.x;

											ysl::Point3i buf_point = { buf_x,buf_y, buf_z };

											//边界数据全部加上
											auto distance = buf_point - global_point;

											//if (distance.Length() > max_distance) continue;

											if (buf_index < 0 || buf_index >= block_size.Prod() || block_mask_vector[buf_index]) continue;

											if (block_mask_vector[buf_index] == 0)
											{
												/*if (buf_point.x < padding || buf_point.x >= block_size.x - padding ||
													buf_point.y < padding || buf_point.y >= block_size.y - padding ||
													buf_point.z < padding || buf_point.z >= block_size.z - padding)
												{*/
												if (buf_point.x < 0 || buf_point.x >= block_size.x ||
													buf_point.y < 0 || buf_point.y >= block_size.y ||
													buf_point.z < 0 || buf_point.z >= block_size.z)
												{
													//设定padding部分数据为原始数据
													//index_stack.push(buf_index);
													//block_mask_vector[buf_index] = 1;
												}
												else if (block_data[buf_index] >= threshold-5)
												{
													index_stack.push(buf_index);
													block_mask_vector[buf_index] = 1;
												}
											}
										}



										//vm::println("test0");
									}



									
								}
								
							}

							block_writer.writeBody(block_vector.data(), sizeof(unsigned char) * block_size.Prod());

							int num = 0;
							for (auto p = 0; p < block_size.Prod(); p++) if (block_vector[p]) num++;

							vm::println("Non-empty voxels number is changed to {}", num);
							
						}
						else
						{
							block_writer.writeBody(block_data, sizeof(unsigned char)* block_size.Prod());
						}
						
						

						//int num = 0;
						//for (auto p = 0; p < block_size.Prod(); p++) if (block_data[p]) num++;

						
						
					}
				}
			}
		}

		
		
	}
	catch (std::exception & e)
	{
		vm::println("{}", e.what());
	}
	

	
}
