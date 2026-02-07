#pragma once

#include "stream.h"

#include <strmd/deserializer.h>
#include <strmd/packer.h>
#include <strmd/serializer.h>

namespace coipc
{
	typedef strmd::varint packer;

	typedef strmd::serializer<buffer_writer< std::vector<std::uint8_t> >, packer> serializer;
	typedef strmd::deserializer<buffer_reader, packer> deserializer;
}
