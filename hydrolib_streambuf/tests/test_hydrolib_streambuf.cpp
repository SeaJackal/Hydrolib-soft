#include <gtest/gtest.h>

#include <deque>
#include <istream>
#include <ostream>
#include <string>
#include <vector>

#include "hydrolib_streambuf.hpp"

namespace {

class FakeStream {
 public:
  FakeStream() = default;

  explicit FakeStream(const std::string &initial_data) { load(initial_data); }

  void load(const std::string &data) {
    input_.clear();
    for (char ch : data) {
      input_.push_back(ch);
    }
  }

  const std::vector<char> &writtenData() const { return written_; }

  void failWrites(bool value = true) { fail_writes_ = value; }

  friend int write(FakeStream &stream, const void *source, unsigned length);
  friend int read(FakeStream &stream, void *dest, unsigned length);

 private:
  std::deque<char> input_;
  std::vector<char> written_;
  bool fail_writes_ = false;
};

[[maybe_unused]] int write(FakeStream &stream, const void *source,
                           unsigned length) {
  if (stream.fail_writes_) {
    return 0;
  }

  const auto *bytes = static_cast<const char *>(source);
  stream.written_.insert(stream.written_.end(), bytes, bytes + length);
  return static_cast<int>(length);
}

[[maybe_unused]] int read(FakeStream &stream, void *dest, unsigned length) {
  auto *bytes = static_cast<char *>(dest);
  unsigned count = 0;

  while (count < length && !stream.input_.empty()) {
    bytes[count++] = stream.input_.front();
    stream.input_.pop_front();
  }

  return static_cast<int>(count);
}

}  // namespace

TEST(HydrolibStreambufTest, OverflowWritesSingleCharacter) {
  FakeStream stream;
  hydrolib::Streambuf<FakeStream> streambuf(stream);

  EXPECT_EQ(streambuf.overflow('a'), 'a');
  ASSERT_EQ(stream.writtenData().size(), 1u);
  EXPECT_EQ(stream.writtenData()[0], 'a');
}

TEST(HydrolibStreambufTest, OverflowReturnsEofWhenArgumentIsEof) {
  FakeStream stream;
  hydrolib::Streambuf<FakeStream> streambuf(stream);

  EXPECT_EQ(streambuf.overflow(std::char_traits<char>::eof()),
            std::char_traits<char>::eof());
  EXPECT_TRUE(stream.writtenData().empty());
}

TEST(HydrolibStreambufTest, OverflowReturnsEofWhenWriteFails) {
  FakeStream stream;
  stream.failWrites();
  hydrolib::Streambuf<FakeStream> streambuf(stream);

  EXPECT_EQ(streambuf.overflow('a'), std::char_traits<char>::eof());
  EXPECT_TRUE(stream.writtenData().empty());
}

TEST(HydrolibStreambufTest, UflowClearsCachedCharacter) {
  FakeStream stream("ab");
  hydrolib::Streambuf<FakeStream> streambuf(stream);

  EXPECT_EQ(streambuf.underflow(), 'a');
  EXPECT_EQ(streambuf.uflow(), 'a');
  EXPECT_EQ(streambuf.underflow(), 'b');
}

TEST(HydrolibStreambufTest, UnderflowReturnsEofWhenNoData) {
  FakeStream stream;
  hydrolib::Streambuf<FakeStream> streambuf(stream);

  EXPECT_EQ(streambuf.underflow(), std::char_traits<char>::eof());
}

TEST(HydrolibStreambufTest, UflowReturnsEofWhenNoData) {
  FakeStream stream;
  hydrolib::Streambuf<FakeStream> streambuf(stream);

  EXPECT_EQ(streambuf.uflow(), std::char_traits<char>::eof());
}

TEST(HydrolibStreambufTest, IstreamReadsThroughHydrolibStreambuf) {
  FakeStream stream("abc");
  hydrolib::Streambuf<FakeStream> streambuf(stream);
  std::istream in(&streambuf);

  std::string result(3, '\0');
  in.read(result.data(), static_cast<std::streamsize>(result.size()));
  EXPECT_EQ(result, "abc");

  EXPECT_EQ(in.get(), std::char_traits<char>::eof());
}

TEST(HydrolibStreambufTest, IstreamReportsEofWhenEmpty) {
  FakeStream stream;
  hydrolib::Streambuf<FakeStream> streambuf(stream);
  std::istream in(&streambuf);

  EXPECT_EQ(in.get(), std::char_traits<char>::eof());
  EXPECT_TRUE(in.eof());
}

TEST(HydrolibStreambufTest, OstreamWritesThroughHydrolibStreambuf) {
  FakeStream stream;
  hydrolib::Streambuf<FakeStream> streambuf(stream);
  std::ostream out(&streambuf);

  out << 'x' << 'y' << 'z';
  out.flush();

  ASSERT_EQ(stream.writtenData().size(), 3u);
  EXPECT_EQ(stream.writtenData()[0], 'x');
  EXPECT_EQ(stream.writtenData()[1], 'y');
  EXPECT_EQ(stream.writtenData()[2], 'z');
}

TEST(HydrolibStreambufTest, OstreamSetsFailbitWhenUnderlyingWriteFails) {
  FakeStream stream;
  stream.failWrites();
  hydrolib::Streambuf<FakeStream> streambuf(stream);
  std::ostream out(&streambuf);

  out << 'a';

  EXPECT_TRUE(out.fail());
  EXPECT_TRUE(stream.writtenData().empty());
}
