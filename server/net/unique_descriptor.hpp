#pragma once

namespace hypercom::server {

// Ownership of a POSIX file descriptor.
//
// Declaring the move constructor implicitly deletes the copy: a descriptor
// can therefore never be duplicated by accident, which avoids a double
// close -- and especially closing a descriptor that has since been
// reassigned to another connection.
class unique_descriptor {
public:
    explicit unique_descriptor(int descriptor = -1) noexcept;

    ~unique_descriptor();

    unique_descriptor(unique_descriptor &&other) noexcept;

    unique_descriptor &operator=(unique_descriptor &&other) noexcept;

    [[nodiscard]] int get_value() const noexcept;

private:
    void close_descriptor() noexcept;

    int descriptor_;
};

} // namespace hypercom::server
