#include "client/keystore/sealed_file.hpp"

#include <sys/stat.h>

#include <filesystem>
#include <fstream>

namespace hypercom::client {

bool write_sealed_file(std::string const &path,
                       std::span<std::uint8_t const> sealed,
                       std::string &error_out)
{
    std::error_code failure;
    std::filesystem::path const target{path};
    if (target.has_parent_path()) {
        std::filesystem::create_directories(target.parent_path(), failure);
    }
    std::ofstream output{path, std::ios::binary | std::ios::trunc};
    if (!output) {
        error_out = "ecriture impossible : " + path;
        return false;
    }
    output.write(reinterpret_cast<char const *>(sealed.data()),
                 static_cast<std::streamsize>(sealed.size()));
    output.close();
#if defined(_WIN32)
    std::filesystem::permissions(target,
                                 std::filesystem::perms::owner_read
                                     | std::filesystem::perms::owner_write,
                                 failure);
#else
    if (::chmod(path.c_str(), S_IRUSR | S_IWUSR) != 0) {
        error_out = "chmod 0600 impossible sur " + path;
        return false;
    }
#endif
    return true;
}

bool read_sealed_file(std::string const &path, std::vector<std::uint8_t> &out)
{
    std::ifstream input{path, std::ios::binary | std::ios::ate};
    if (!input) {
        return false;
    }
    std::streamsize const size = input.tellg();
    if (size <= 0) {
        return false;
    }
    out.resize(static_cast<std::size_t>(size));
    input.seekg(0);
    input.read(reinterpret_cast<char *>(out.data()), size);
    return input.gcount() == size;
}

} // namespace hypercom::client
