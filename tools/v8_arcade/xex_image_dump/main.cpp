#include <rex/kernel/init.h>
#include <rex/runtime.h>
#include <rex/system/kernel_state.h>
#include <rex/system/user_module.h>
#include <rex/system/xex_module.h>
#include <rex/system/xtypes.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static bool WriteFile(const fs::path& path, const uint8_t* data, size_t size) {
  std::ofstream output(path, std::ios::binary);
  if (!output) {
    std::cerr << "Could not open " << path << " for writing\n";
    return false;
  }
  output.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
  return output.good();
}

static std::string SafeSectionName(std::string name) {
  for (char& value : name) {
    if (!(value >= 'A' && value <= 'Z') && !(value >= 'a' && value <= 'z') &&
        !(value >= '0' && value <= '9') && value != '.' && value != '_' && value != '-') {
      value = '_';
    }
  }
  return name.empty() ? "unnamed" : name;
}

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cerr << "Usage: v8_arcade_xex_image_dump <game-root> <guest-xex-path> <output-dir>\n";
    return 2;
  }

  const fs::path gameRoot = fs::weakly_canonical(argv[1]);
  const std::string guestPath = argv[2];
  const fs::path outputRoot = fs::absolute(argv[3]);
  fs::create_directories(outputRoot / "sections");

  rex::Runtime runtime(gameRoot);
  rex::RuntimeConfig config{
      .kernel_init = rex::kernel::InitializeKernel,
      .tool_mode = true,
  };
  rex::X_STATUS status = runtime.Setup(std::move(config));
  if (status != 0) {
    std::cerr << "Runtime setup failed: 0x" << std::hex << status << "\n";
    return 3;
  }

  status = runtime.LoadXexImage(guestPath);
  if (status != 0) {
    std::cerr << "XEX load failed: 0x" << std::hex << status << "\n";
    return 4;
  }

  auto userModule = runtime.kernel_state()->GetExecutableModule();
  if (!userModule || !userModule->xex_module()) {
    std::cerr << "Runtime did not expose the executable XEX module\n";
    return 5;
  }

  auto* module = userModule->xex_module();
  const uint32_t imageBase = module->base_address();
  const uint32_t imageSize = module->image_size();
  std::vector<uint8_t> image(imageSize, 0);

  std::ofstream manifest(outputRoot / "sections.tsv");
  manifest << "name\tvirtual_address\tvirtual_size\texecutable\twritable\tfile\n";

  for (const auto& section : module->binary_sections()) {
    if (!section.host_data || !section.virtual_size || section.virtual_address < imageBase ||
        uint64_t(section.virtual_address) + section.virtual_size >
            uint64_t(imageBase) + imageSize) {
      std::cerr << "Skipping invalid section " << section.name << "\n";
      continue;
    }

    const size_t imageOffset = section.virtual_address - imageBase;
    std::copy_n(section.host_data, section.virtual_size, image.data() + imageOffset);

    std::ostringstream address;
    address << std::hex << std::uppercase << std::setw(8) << std::setfill('0')
            << section.virtual_address;
    const std::string fileName = SafeSectionName(section.name) + "_" + address.str() + ".bin";
    if (!WriteFile(outputRoot / "sections" / fileName, section.host_data, section.virtual_size)) {
      return 6;
    }

    manifest << section.name << "\t0x" << address.str() << "\t0x" << std::hex
             << std::uppercase << section.virtual_size << std::dec << "\t"
             << (section.executable ? 1 : 0) << "\t" << (section.writable ? 1 : 0) << "\t"
             << "sections/" << fileName << "\n";
  }

  std::ostringstream imageAddress;
  imageAddress << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << imageBase;
  const fs::path imagePath = outputRoot / ("image_" + imageAddress.str() + ".bin");
  if (!WriteFile(imagePath, image.data(), image.size())) {
    return 7;
  }

  std::cout << "Loaded XEX at 0x" << imageAddress.str() << ", image size 0x" << std::hex
            << std::uppercase << imageSize << ", entry point 0x" << module->entry_point() << "\n";
  std::cout << "Wrote " << imagePath << " and " << std::dec
            << module->binary_sections().size()
            << " section files\n";
  return 0;
}
