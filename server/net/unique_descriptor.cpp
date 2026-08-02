#include "server/net/unique_descriptor.hpp"

#if defined(_WIN32)
#include <winsock2.h>
#else
#include <unistd.h>
#endif

#include <utility>

namespace hypercom::server {

unique_descriptor::unique_descriptor(int descriptor) noexcept
    : descriptor_{descriptor}
{
}

unique_descriptor::~unique_descriptor()
{
    close_descriptor();
}

unique_descriptor::unique_descriptor(unique_descriptor &&other) noexcept
    : descriptor_{std::exchange(other.descriptor_, -1)}
{
}

unique_descriptor &
unique_descriptor::operator=(unique_descriptor &&other) noexcept
{
    if (this != &other) {
        close_descriptor();
        descriptor_ = std::exchange(other.descriptor_, -1);
    }
    return *this;
}

int unique_descriptor::get_value() const noexcept
{
    return descriptor_;
}

void unique_descriptor::close_descriptor() noexcept
{
    if (descriptor_ >= 0) {
#if defined(_WIN32)
        ::closesocket(static_cast<SOCKET>(descriptor_));
#else
        ::close(descriptor_);
#endif
        descriptor_ = -1;
    }
}

} // namespace hypercom::server
