#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include "VMCoreExtension/i3dblockfileplugininterface.h"
#include "VMFoundation/pluginloader.h"
#include <VMUtils/log.hpp>
#include <VMat/numeric.h>
class BlockDataReader
{
public:
	BlockDataReader();
	~BlockDataReader();
	bool open(const std::string& file_name);
	void close();

	const void* getBlock(int block_id);
	const void* getBlock(int x, int y, int z);

	int getPadding() const { return reader->GetPadding(); }
	int getBlockBased() const { return reader->Get3DPageSizeInLog(); }
	ysl::Size3 getDataSizeDimension() const { return reader->GetDataSizeWithoutPadding(); }
	ysl::Size3 getBlockSize() const { return reader->Get3DPageSize(); }
	ysl::Size3 getBlockNumber() const { return reader->Get3DPageCount(); }
	size_t getPhysicalPageCount() const { return reader->GetPhysicalPageCount(); }
	size_t getVirtualPageCount() const { return reader->GetVirtualPageCount(); }

private:
	::vm::Ref<I3DBlockFilePluginInterface> reader;
};

inline BlockDataReader::BlockDataReader()
{
	

}

inline BlockDataReader::~BlockDataReader()
{
}

inline bool BlockDataReader::open(const std::string& file_name)
{
	ysl::PluginLoader::GetPluginLoader()->LoadPlugins("plugins");

	const std::string key = ".lvd";
	//const std::string key = ".blk";
	reader = ysl::PluginLoader::GetPluginLoader()->CreatePlugin<I3DBlockFilePluginInterface>(key);

	if (reader == nullptr)
	{
		vm::println("failed to load plugin for {}.", key);
		return false;
	}

	try
	{
		reader->Open(file_name);
	}
	catch (std::exception & e)
	{
		vm::println("{}", e.what());
	}
	const auto pageCount = reader->Get3DPageCount();
	const auto pageSize = reader->Get3DPageSize();
	const auto sizeInLog = reader->Get3DPageSizeInLog();
	const auto dataSize = reader->GetDataSizeWithoutPadding();
	const auto padding = reader->GetPadding();

	vm::println("Number of blocks : \t{}", pageCount);
	vm::println("Size of block : \t{}", pageSize);
	vm::println("Block based : \t{}", sizeInLog);
	vm::println("Data size : \t{}", dataSize);
	vm::println("Block padding : \t{}", padding);



	const auto pages = reader->GetPageSize();
	const auto pPageCount = reader->GetPhysicalPageCount();
	const auto vPageCount = reader->GetVirtualPageCount();

	
	return true;
}

inline void BlockDataReader::close()
{
}

inline const void* BlockDataReader::getBlock(int block_id)
{
	//const auto pageCount = reader->Get3DPageCount();
	//const auto pageSize = reader->Get3DPageSize();
	//const auto sizeInLog = reader->Get3DPageSizeInLog();
	//const auto dataSize = reader->GetDataSizeWithoutPadding();
	//const auto padding = reader->GetPadding();

	//const auto pages = reader->GetPageSize();
	//const auto pPageCount = reader->GetPhysicalPageCount();
	//const auto vPageCount = reader->GetVirtualPageCount();

	return reader->GetPage(block_id);
}

inline const void* BlockDataReader::getBlock(int x,int y,int z)
{
	const auto pageCount = reader->Get3DPageCount();
	const auto pageSize = reader->Get3DPageSize();
	//const auto sizeInLog = reader->Get3DPageSizeInLog();
	//const auto dataSize = reader->GetDataSizeWithoutPadding();
	//const auto padding = reader->GetPadding();
	//
	//const auto pages = reader->GetPageSize();
	//const auto pPageCount = reader->GetPhysicalPageCount();
	//const auto vPageCount = reader->GetVirtualPageCount();
	
	//auto id = ysl::Linear({ x,y,z }, { pageSize.x, pageSize.y });
	auto id = ysl::Linear({x,y,z}, { pageCount.x, pageCount.y});
	return reader->GetPage(id);
}

