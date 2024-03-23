#ifndef READER_HPP
#define READER_HPP

#include "utils/result.hpp"
#include "utils/utils.hpp"
#include "utils/ownership.hpp"
#include <string>

class IReader {
public:
    virtual ~IReader();
    // Read up to n bytes into buf
    // Return the number of bytes read, or an error if one occurred
    virtual Result<std::size_t, std::string> read(char *buf, std::size_t n) = 0;
    virtual bool eof() const = 0;
};

class FdReader : public IReader {
public:
    explicit FdReader(int fd, Ownership ownership = kBorrow);
    // Close the file descriptor if ownership is kOwn
    virtual ~FdReader();

    virtual Result<std::size_t, std::string> read(char *buf, std::size_t n);
    virtual bool eof() const;

private:
    int fd_;
    bool eof_;
    Ownership ownership_;
};

class BufferedReader : public IReader {
public:
    explicit BufferedReader(IReader *reader, Ownership ownership = kBorrow);
    explicit BufferedReader(IReader *reader, std::size_t buffer_size, Ownership ownership = kBorrow);
    // Delete the reader if ownership is kOwn
    virtual ~BufferedReader();

    Result<std::size_t, std::string> read(char *buf, std::size_t n);
    Result<std::string, std::string> readLine(const std::string &delimiter = "\n");
    bool eof() const;

private:
    static const std::size_t kDefaultBufferSize = 4 * utils::kKiB;
    IReader *reader_;
    Ownership ownership_;
    char *buf_;
    std::size_t buf_size_;
    std::size_t buf_read_pos_;
    std::size_t buf_write_pos_;

    Result<std::size_t, std::string> fillBuffer();
    Result<std::string, std::string> readLineFromBuffer(const std::string &delimiter);
};

#endif //READER_HPP
