#pragma once


#include <VMat/geometry.h>
#include <VMCoreExtension/i3dblockfileplugininterface.h>
#include "VMCoreExtension/plugin.h"
#include <VMUtils/vmnew.hpp>
#include <fstream>


class BlkBlockFilePlugin : public ::vm::EverythingBase<I3DBlockFilePluginInterface>
{
public:
	BlkBlockFilePlugin(::vm::IRefCnt* cnt): ::vm::EverythingBase<I3DBlockFilePluginInterface>(cnt){};
	virtual void Open(const std::string& fileName);
	virtual int GetPadding() const;
	virtual ysl::Size3 GetDataSizeWithoutPadding() const;
	virtual ysl::Size3 Get3DPageSize() const;
	virtual int Get3DPageSizeInLog() const;
	virtual ysl::Size3 Get3DPageCount() const;

	virtual const void* GetPage(size_t pageID);

	virtual size_t GetPageSize() const;

	virtual size_t GetPhysicalPageCount() const ;

	virtual size_t GetVirtualPageCount()const ;

private:

	std::ifstream inFile;

	char* dataPtr = nullptr;

	ysl::Size3 blockSize;

	ysl::Size3 volume_size;
	size_t page_size, physical_page_count, virtual_page_count, block_based, padding;
	ysl::Size3 block_number;
	size_t head_size = 112;

	size_t non_empty_block_number = 0;
	std::vector<size_t> non_empty_block_id_array;
	unsigned char* empty_data_ptr = nullptr;
};

class BlkBlockFileReaderFactory :public ysl::IPluginFactory
{
public:
	DECLARE_PLUGIN_FACTORY("visualman.blockdata.io")
	::vm::IEverything* Create(const std::string& key) override
	{
		if (key == Keys()[0]) return VM_NEW<BlkBlockFilePlugin>();
		return nullptr;
	};
	std::vector<std::string> Keys() const override { return { ".blk" }; };
};


EXPORT_PLUGIN_FACTORY(BlkBlockFileReaderFactory)
//DECLARE_PLUGIN_METADATA(BlkBlockFilePlugin, "visualman.blockdata.io")
