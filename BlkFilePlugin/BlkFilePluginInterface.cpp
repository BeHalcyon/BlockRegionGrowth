

#include "BlkFilePluginInterface.h"
#include "VMFoundation/pluginloader.h"
#include "VMCoreExtension/ifilemappingplugininterface.h"
#include <fstream>
#include <VMUtils/log.hpp>

void BlkBlockFilePlugin::Open(const std::string& fileName)
{

	inFile.open(fileName, std::ios::binary);
	if (inFile.is_open() == false)
	{
		throw std::runtime_error("can not file file");
	}

	//inFile.seekg(0, std::ios::end);

	//const size_t fileBytes = inFile.tellg();
	//inFile.close();

	inFile.read((char*)&volume_size.x, sizeof(size_t));
	inFile.read((char*)&volume_size.y, sizeof(size_t));
	inFile.read((char*)&volume_size.z, sizeof(size_t));


	inFile.read((char*)&block_number.x, sizeof(size_t));
	inFile.read((char*)&block_number.y, sizeof(size_t));
	inFile.read((char*)&block_number.z, sizeof(size_t));
	
	inFile.read((char*)&blockSize.x, sizeof(size_t));
	inFile.read((char*)&blockSize.y, sizeof(size_t));
	inFile.read((char*)&blockSize.z, sizeof(size_t));


	inFile.read((char*)&page_size, sizeof(size_t));
	inFile.read((char*)&physical_page_count, sizeof(size_t));
	inFile.read((char*)&virtual_page_count, sizeof(size_t));
	inFile.read((char*)&block_based, sizeof(size_t));
	inFile.read((char*)&padding, sizeof(size_t));


	inFile.read((char*)&non_empty_block_number, sizeof(size_t));
	non_empty_block_id_array.resize(non_empty_block_number, -1);
	for(auto i=0;i<non_empty_block_number;i++)
	{
		inFile.read((char*)&non_empty_block_id_array[i], sizeof(size_t));
		//vm::println("non_empty_block_id_array[{}]:{}", i, non_empty_block_id_array[i]);
	}

	head_size = 112 + (1 + non_empty_block_number) * 8;
	
	dataPtr = new char[blockSize.Prod()];
	
	empty_data_ptr = new unsigned char[blockSize.Prod()];
	for (auto i = 0; i < blockSize.Prod(); i++) empty_data_ptr[i] = 0;
}

int BlkBlockFilePlugin::GetPadding() const
{
	return padding;
}

ysl::Size3 BlkBlockFilePlugin::GetDataSizeWithoutPadding() const
{
	return volume_size;
}

ysl::Size3 BlkBlockFilePlugin::Get3DPageSize() const
{
	return blockSize;
}

int BlkBlockFilePlugin::Get3DPageSizeInLog() const
{
	return block_based;
}

ysl::Size3 BlkBlockFilePlugin::Get3DPageCount() const
{
	return block_number;
}

const void* BlkBlockFilePlugin::GetPage(size_t pageID)
{
	
	auto index = -1;
	for(auto i=0;i<non_empty_block_number;i++)
	{
		if(non_empty_block_id_array[i]==pageID)
		{
			index = i;
			break;
		}
	}
	//empty block
	if(index == -1)
	{
		return empty_data_ptr;
	}
	//non-empty block
	else
	{
		inFile.seekg(head_size + index * blockSize.Prod(), std::ios::beg);
		inFile.read(dataPtr, blockSize.Prod());
		return dataPtr;
	}
	
}

size_t BlkBlockFilePlugin::GetPageSize() const
{
	return page_size;
}

size_t BlkBlockFilePlugin::GetPhysicalPageCount() const
{
	return physical_page_count;
}

size_t BlkBlockFilePlugin::GetVirtualPageCount() const
{
	return virtual_page_count;
}

EXPORT_PLUGIN_FACTORY_IMPLEMENT(BlkBlockFileReaderFactory)
