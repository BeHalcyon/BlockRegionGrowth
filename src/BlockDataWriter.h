#pragma once
#include <fstream>
#include <VMat/numeric.h>
class BlockDataWriter
{
public:
	BlockDataWriter();
	bool open(const std::string & block_data_file);
	void writeHead(const ysl::Size3 & data);
	void writeHead(int data, int byte_size);
	void writeHead(const size_t data);
	void writeBody(const unsigned char* data, const size_t size);

	void close();
	
private:
	std::ofstream writer;
	
};

inline BlockDataWriter::BlockDataWriter()
{
}

inline bool BlockDataWriter::open(const std::string& block_data_file)
{
	writer.open(block_data_file, std::ios::binary);
	return writer.is_open();
}

inline void BlockDataWriter::writeHead(const ysl::Size3& data)
{
	writer.write((char*)&data.x, sizeof(size_t));
	writer.write((char*)&data.y, sizeof(size_t));
	writer.write((char*)&data.z, sizeof(size_t));
}

inline void BlockDataWriter::writeHead(const size_t data)
{
	writer.write((char*)&data, sizeof(size_t));
}

inline void BlockDataWriter::writeBody(const unsigned char* data, const size_t size)
{
	writer.write((char*)data, sizeof(unsigned char)*size);
}

inline void BlockDataWriter::close()
{
	
}
