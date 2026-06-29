#pragma once

#include <cstring>
#include <string_view>

#include "hydrolib_stream_concepts.hpp"

namespace hydrolib::shell {
class StreamInterface {  // NOLINT
 public:
  virtual ~StreamInterface() = default;

  virtual int Read(void* dest, unsigned length) = 0;
  virtual int Write(const void* source, unsigned length) = 0;
};

template <concepts::stream::ByteFullStreamConcept Stream>
class StreamWrapper : public StreamInterface {
 public:
  explicit StreamWrapper(Stream& stream) : stream_(stream) {}
  StreamWrapper(const StreamWrapper&) = delete;
  StreamWrapper(StreamWrapper&&) = delete;
  StreamWrapper& operator=(const StreamWrapper&) = delete;
  StreamWrapper& operator=(StreamWrapper&&) = delete;
  ~StreamWrapper() override = default;

  int Read(void* dest, unsigned length) override {
    return read(stream_, dest, length);
  }
  int Write(const void* source, unsigned length) override {
    return write(stream_, source, length);
  }

 private:
  Stream& stream_;
};

class Ostream {
 public:
  explicit Ostream(StreamInterface& stream) : stream_(stream) {}
  Ostream(const Ostream&) = delete;
  Ostream(Ostream&&) = delete;
  Ostream& operator=(const Ostream&) = delete;
  Ostream& operator=(Ostream&&) = delete;
  ~Ostream() = default;

  Ostream& operator<<(std::string_view string) {
    stream_.Write(string.data(), static_cast<unsigned>(string.length()));
    return *this;
  }

  Ostream& operator<<(char chr) {
    stream_.Write(&chr, 1);
    return *this;
  }

 private:
  StreamInterface& stream_;
};

}  // namespace hydrolib::shell
