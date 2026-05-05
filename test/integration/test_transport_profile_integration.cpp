#include "dlms/transport/byte_stream.hpp"
#include "dlms/transport/datagram_transport.hpp"
#include "dlms/transport/fake_transport.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>

namespace {

using dlms::transport::FakeByteStream;
using dlms::transport::FakeDatagramTransport;
using dlms::transport::IByteStream;
using dlms::transport::IDatagramTransport;
using dlms::transport::TransportStatus;

class WrapperTcpProfileChannel
{
public:
  explicit WrapperTcpProfileChannel(IByteStream& stream)
    : stream_(stream)
  {
  }

  TransportStatus SendWpdu(const std::uint8_t* data, std::size_t size)
  {
    return stream_.WriteAll(data, size);
  }

private:
  IByteStream& stream_;
};

class WrapperUdpProfileChannel
{
public:
  explicit WrapperUdpProfileChannel(IDatagramTransport& datagram)
    : datagram_(datagram)
  {
  }

  TransportStatus SendWpdu(const std::uint8_t* data, std::size_t size)
  {
    return datagram_.Send(data, size);
  }

private:
  IDatagramTransport& datagram_;
};

class HdlcProfileChannel
{
public:
  explicit HdlcProfileChannel(IByteStream& stream)
    : stream_(stream)
  {
  }

  TransportStatus SendFrame(const std::uint8_t* data, std::size_t size)
  {
    return stream_.WriteAll(data, size);
  }

private:
  IByteStream& stream_;
};

TEST(TransportProfileIntegration, WrapperTcpProfileCanDependOnByteStream)
{
  FakeByteStream stream;
  WrapperTcpProfileChannel channel(stream);
  const std::uint8_t wpdu[] = { 0x00, 0x01, 0x00, 0x01 };

  ASSERT_EQ(TransportStatus::Ok, stream.Open());
  EXPECT_EQ(TransportStatus::Ok, channel.SendWpdu(wpdu, sizeof(wpdu)));
  ASSERT_EQ(1u, stream.Writes().size());
  EXPECT_EQ(4u, stream.Writes()[0].size());
}

TEST(TransportProfileIntegration, WrapperUdpProfileCanDependOnDatagramTransport)
{
  FakeDatagramTransport datagram;
  WrapperUdpProfileChannel channel(datagram);
  const std::uint8_t wpdu[] = { 0x00, 0x01, 0x00, 0x01 };

  ASSERT_EQ(TransportStatus::Ok, datagram.Open());
  EXPECT_EQ(TransportStatus::Ok, channel.SendWpdu(wpdu, sizeof(wpdu)));
  ASSERT_EQ(1u, datagram.SentDatagrams().size());
  EXPECT_EQ(4u, datagram.SentDatagrams()[0].size());
}

TEST(TransportProfileIntegration, HdlcProfileCanDependOnByteStream)
{
  FakeByteStream stream;
  HdlcProfileChannel channel(stream);
  const std::uint8_t frame[] = { 0x7e, 0xa0, 0x03, 0x7e };

  ASSERT_EQ(TransportStatus::Ok, stream.Open());
  EXPECT_EQ(TransportStatus::Ok, channel.SendFrame(frame, sizeof(frame)));
  ASSERT_EQ(1u, stream.Writes().size());
  EXPECT_EQ(4u, stream.Writes()[0].size());
}

} // namespace