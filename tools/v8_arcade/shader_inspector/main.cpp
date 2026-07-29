#include <bit>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _MSC_VER
#include <cmath>
#define __builtin_isnan(value) std::isnan(value)
#endif

#include <rex/graphics/pipeline/shader/shader.h>
#include <rex/string/buffer.h>

namespace {

uint32_t LoadU32LE(const std::vector<uint8_t>& data, size_t offset) {
  if (offset + 4 > data.size()) {
    throw std::runtime_error("read outside input");
  }
  return uint32_t(data[offset]) | (uint32_t(data[offset + 1]) << 8) |
         (uint32_t(data[offset + 2]) << 16) |
         (uint32_t(data[offset + 3]) << 24);
}

uint32_t LoadU32BE(const std::vector<uint8_t>& data, size_t offset) {
  if (offset + 4 > data.size()) {
    throw std::runtime_error("read outside input");
  }
  return (uint32_t(data[offset]) << 24) |
         (uint32_t(data[offset + 1]) << 16) |
         (uint32_t(data[offset + 2]) << 8) | uint32_t(data[offset + 3]);
}

std::vector<uint8_t> ReadFile(const std::filesystem::path& path) {
  std::ifstream stream(path, std::ios::binary | std::ios::ate);
  if (!stream) {
    throw std::runtime_error("could not open input");
  }
  const auto size = stream.tellg();
  stream.seekg(0);
  std::vector<uint8_t> result(static_cast<size_t>(size));
  stream.read(reinterpret_cast<char*>(result.data()), size);
  if (!stream) {
    throw std::runtime_error("could not read input");
  }
  return result;
}

void InspectTechnique(const std::vector<uint8_t>& data, uint32_t technique) {
  const uint32_t vertex_vector = LoadU32LE(data, technique);
  const uint32_t pixel_vector = LoadU32LE(data, technique + 4);
  const uint32_t vertex_data = LoadU32LE(data, vertex_vector);
  const uint32_t vertex_blob_dwords = LoadU32LE(data, vertex_vector + 4);
  const uint32_t pixel_data = LoadU32LE(data, pixel_vector);
  const uint32_t pixel_blob_dwords = LoadU32LE(data, pixel_vector + 4);
  if (vertex_data + vertex_blob_dwords * 4 > data.size() ||
      pixel_data + pixel_blob_dwords * 4 > data.size()) {
    throw std::runtime_error("shader vector outside input");
  }

  // Xbox 360 compiled shaders are containers.  The Shader record describes
  // raw GPU microcode stored in the physical section following virtualSize.
  const uint32_t flags = LoadU32BE(data, vertex_data);
  if ((flags & 0xFFFFFF00u) != 0x102A1100u) {
    throw std::runtime_error("vertex shader has invalid container magic");
  }
  const uint32_t virtual_size = LoadU32BE(data, vertex_data + 4);
  const uint32_t shader_offset = LoadU32BE(data, vertex_data + 24);
  const uint32_t shader = vertex_data + shader_offset;
  const uint32_t physical_offset = LoadU32BE(data, shader);
  const uint32_t ucode_dwords = LoadU32BE(data, shader + 4);
  const uint32_t ucode_data = vertex_data + virtual_size + physical_offset;
  if (ucode_data + ucode_dwords * 4 > data.size()) {
    throw std::runtime_error("vertex microcode outside input");
  }

  rex::string::StringBuffer disassembly_buffer;
  rex::graphics::Shader vertex_shader(
      rex::graphics::xenos::ShaderType::kVertex, technique,
      reinterpret_cast<const uint32_t*>(data.data() + ucode_data),
      ucode_dwords / 4, std::endian::big);
  vertex_shader.AnalyzeUcode(disassembly_buffer);

  std::cout << "technique=0x" << std::hex << technique
            << " vertex_blob=0x" << vertex_data << " ucode=0x" << ucode_data
            << std::dec << " ucode_dwords=" << ucode_dwords << "\n";
  for (const auto& binding : vertex_shader.vertex_bindings()) {
    std::cout << " binding=" << binding.binding_index
              << " fetch_constant=" << binding.fetch_constant
              << " stride_bytes=" << binding.stride_words * 4 << "\n";
    for (const auto& attribute : binding.attributes) {
      const auto& fetch = attribute.fetch_instr;
      std::cout << "  offset_bytes=" << fetch.attributes.offset * 4
                << " format="
                << static_cast<uint32_t>(fetch.attributes.data_format)
                << " signed=" << fetch.attributes.is_signed
                << " integer=" << fetch.attributes.is_integer
                << " result_register=" << fetch.result.storage_index << "\n";
    }
  }
  std::cout << vertex_shader.ucode_disassembly() << "\n";

  const uint32_t pixel_flags = LoadU32BE(data, pixel_data);
  if ((pixel_flags & 0xFFFFFF00u) != 0x102A1100u) {
    throw std::runtime_error("pixel shader has invalid container magic");
  }
  const uint32_t pixel_virtual_size = LoadU32BE(data, pixel_data + 4);
  const uint32_t pixel_shader_offset = LoadU32BE(data, pixel_data + 24);
  const uint32_t pixel_record = pixel_data + pixel_shader_offset;
  const uint32_t pixel_physical_offset = LoadU32BE(data, pixel_record);
  const uint32_t pixel_ucode_dwords = LoadU32BE(data, pixel_record + 4);
  const uint32_t pixel_ucode_data =
      pixel_data + pixel_virtual_size + pixel_physical_offset;
  if (pixel_ucode_data + pixel_ucode_dwords * 4 > data.size()) {
    throw std::runtime_error("pixel microcode outside input");
  }

  rex::graphics::Shader pixel_shader(
      rex::graphics::xenos::ShaderType::kPixel, technique,
      reinterpret_cast<const uint32_t*>(data.data() + pixel_ucode_data),
      pixel_ucode_dwords / 4, std::endian::big);
  pixel_shader.AnalyzeUcode(disassembly_buffer);
  std::cout << "pixel_blob=0x" << std::hex << pixel_data << " ucode=0x"
            << pixel_ucode_data << std::dec
            << " ucode_dwords=" << pixel_ucode_dwords << "\n";
  for (const auto& binding : pixel_shader.texture_bindings()) {
    std::cout << " texture_binding=" << binding.binding_index
              << " fetch_constant=" << binding.fetch_constant << "\n";
  }
  std::cout << pixel_shader.ucode_disassembly() << "\n";
}

}  // namespace

int main(int argc, char** argv) {
  try {
    if (argc < 3) {
      std::cerr << "usage: v8_arcade_shader_inspector BUNDLE TECHNIQUE...\n";
      return 2;
    }
    const auto data = ReadFile(argv[1]);
    for (int i = 2; i < argc; ++i) {
      InspectTechnique(data,
                       static_cast<uint32_t>(std::stoul(argv[i], nullptr, 0)));
    }
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << "\n";
    return 1;
  }
}
