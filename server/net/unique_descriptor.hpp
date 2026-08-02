#pragma once

namespace hypercom::server {

// Possession d'un descripteur de fichier POSIX.
//
// Declarer le constructeur de deplacement supprime implicitement la copie :
// un descripteur ne peut donc pas etre duplique par accident, ce qui evite la
// double fermeture -- et surtout la fermeture d'un descripteur qui a entre
// temps ete reattribue a une autre connexion.
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
