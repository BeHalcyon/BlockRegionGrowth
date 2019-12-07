#pragma once
#include "VMUtils/json_binding.hpp"

struct BlockNumberJsonStruct : public vm::json::Serializable<BlockNumberJsonStruct>
{
	VM_JSON_FIELD(std::string, obj_file);
	VM_JSON_FIELD(std::vector<std::vector<int>>, block_number_array);
	VM_JSON_FIELD(std::vector<int>, block_max_bounding);
	VM_JSON_FIELD(std::vector<int>, block_min_bounding);
	VM_JSON_FIELD(int, block_based_number);
	VM_JSON_FIELD(int, padding);
};


struct OutputJsonStruct : public vm::json::Serializable<OutputJsonStruct>
{
	VM_JSON_FIELD(std::vector<BlockNumberJsonStruct>, blocks);
};

struct InputJsonStruct : public vm::json::Serializable<InputJsonStruct>
{
	VM_JSON_FIELD(std::vector<std::string>, obj_files);
	VM_JSON_FIELD(std::string, file_prefix);
};
