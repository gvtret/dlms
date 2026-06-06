#include "dlms/hdlc/hdlc_c_api.h"

#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

namespace {

dlms_hdlc_frame_t MakeSnrmFrame()
{
  dlms_hdlc_frame_t frame;
  frame.segmented = 0u;
  frame.destination_address_raw = 0x01u;
  frame.destination_address_size = 1u;
  frame.source_address_raw = 0x10u;
  frame.source_address_size = 1u;
  frame.control = 0x93u;
  frame.information_data = 0;
  frame.information_size = 0u;
  return frame;
}

dlms_hdlc_frame_t MakeInformationFrame(const std::uint8_t* information,
                                       std::size_t informationSize)
{
  dlms_hdlc_frame_t frame;
  frame.segmented = 0u;
  frame.destination_address_raw = 0x01u;
  frame.destination_address_size = 1u;
  frame.source_address_raw = 0x10u;
  frame.source_address_size = 1u;
  frame.control = 0x10u;
  frame.information_data = information;
  frame.information_size = informationSize;
  return frame;
}

dlms_hdlc_frame_t FilledFrame()
{
  dlms_hdlc_frame_t frame;
  frame.segmented = 1u;
  frame.destination_address_raw = 99u;
  frame.destination_address_size = 4u;
  frame.source_address_raw = 99u;
  frame.source_address_size = 4u;
  frame.control = 99u;
  frame.information_data = reinterpret_cast<const std::uint8_t*>(1);
  frame.information_size = 99u;
  return frame;
}

void ExpectClearedFrame(const dlms_hdlc_frame_t& frame)
{
  EXPECT_EQ(0u, frame.segmented);
  EXPECT_EQ(0u, frame.destination_address_raw);
  EXPECT_EQ(0u, frame.destination_address_size);
  EXPECT_EQ(0u, frame.source_address_raw);
  EXPECT_EQ(0u, frame.source_address_size);
  EXPECT_EQ(0u, frame.control);
  EXPECT_EQ(static_cast<const std::uint8_t*>(0), frame.information_data);
  EXPECT_EQ(0u, frame.information_size);
}

void EncodeFrameOrFail(const dlms_hdlc_frame_t& frame,
                       std::uint8_t* output,
                       std::size_t outputSize,
                       std::size_t* writtenSize)
{
  ASSERT_EQ(DLMS_HDLC_STATUS_OK,
            dlms_hdlc_encode_frame(&frame,
                                   0,
                                   output,
                                   outputSize,
                                   writtenSize));
}

TEST(HdlcCApi, StatusValuesMatchStableAbi)
{
  EXPECT_EQ(0, DLMS_HDLC_STATUS_OK);
  EXPECT_EQ(1, DLMS_HDLC_STATUS_NEED_MORE_DATA);
  EXPECT_EQ(2, DLMS_HDLC_STATUS_OUTPUT_BUFFER_TOO_SMALL);
  EXPECT_EQ(3, DLMS_HDLC_STATUS_INVALID_ARGUMENT);
  EXPECT_EQ(20, DLMS_HDLC_STATUS_INTERNAL_ERROR);
}

TEST(HdlcCApi, EncodeFrame)
{
  const dlms_hdlc_frame_t frame = MakeSnrmFrame();
  std::uint8_t output[32] = {};
  std::size_t writtenSize = 0u;

  ASSERT_EQ(DLMS_HDLC_STATUS_OK,
            dlms_hdlc_encode_frame(&frame,
                                   0,
                                   output,
                                   sizeof(output),
                                   &writtenSize));
  EXPECT_EQ(9u, writtenSize);
  EXPECT_EQ(0x7eu, output[0]);
  EXPECT_EQ(0x7eu, output[writtenSize - 1u]);
}

TEST(HdlcCApi, ZeroLimitFieldsKeepDefaults)
{
  const dlms_hdlc_frame_t frame = MakeSnrmFrame();
  dlms_hdlc_limits_t limits;
  limits.maximum_frame_size = 0u;
  limits.maximum_information_field_size = 0u;
  limits.maximum_reassembled_information_size = 0u;
  std::uint8_t output[32] = {};
  std::size_t writtenSize = 0u;

  ASSERT_EQ(DLMS_HDLC_STATUS_OK,
            dlms_hdlc_encode_frame(&frame,
                                   &limits,
                                   output,
                                   sizeof(output),
                                   &writtenSize));
  EXPECT_EQ(9u, writtenSize);
}

TEST(HdlcCApi, DecodeFrame)
{
  const dlms_hdlc_frame_t frame = MakeSnrmFrame();
  std::uint8_t encoded[32] = {};
  std::size_t writtenSize = 0u;
  ASSERT_EQ(DLMS_HDLC_STATUS_OK,
            dlms_hdlc_encode_frame(&frame,
                                   0,
                                   encoded,
                                   sizeof(encoded),
                                   &writtenSize));

  dlms_hdlc_frame_t decoded;
  std::uint8_t information[8] = {};
  std::size_t informationSize = 0u;
  ASSERT_EQ(DLMS_HDLC_STATUS_OK,
            dlms_hdlc_decode_frame(encoded,
                                   writtenSize,
                                   0,
                                   &decoded,
                                   information,
                                   sizeof(information),
                                   &informationSize));
  EXPECT_EQ(0u, decoded.segmented);
  EXPECT_EQ(0x01u, decoded.destination_address_raw);
  EXPECT_EQ(1u, decoded.destination_address_size);
  EXPECT_EQ(0x10u, decoded.source_address_raw);
  EXPECT_EQ(1u, decoded.source_address_size);
  EXPECT_EQ(0x93u, decoded.control);
  EXPECT_EQ(0u, informationSize);
}

TEST(HdlcCApi, DecodeEmptyInformationAllowsNullInformationBuffer)
{
  const dlms_hdlc_frame_t frame = MakeSnrmFrame();
  std::uint8_t encoded[32] = {};
  std::size_t writtenSize = 0u;
  ASSERT_EQ(DLMS_HDLC_STATUS_OK,
            dlms_hdlc_encode_frame(&frame,
                                   0,
                                   encoded,
                                   sizeof(encoded),
                                   &writtenSize));

  dlms_hdlc_frame_t decoded = FilledFrame();
  std::size_t informationSize = 99u;
  ASSERT_EQ(DLMS_HDLC_STATUS_OK,
            dlms_hdlc_decode_frame(encoded,
                                   writtenSize,
                                   0,
                                   &decoded,
                                   0,
                                   0u,
                                   &informationSize));
  EXPECT_EQ(0x93u, decoded.control);
  EXPECT_EQ(static_cast<const std::uint8_t*>(0), decoded.information_data);
  EXPECT_EQ(0u, decoded.information_size);
  EXPECT_EQ(0u, informationSize);
}

TEST(HdlcCApi, ReportsSmallOutputBuffer)
{
  const dlms_hdlc_frame_t frame = MakeSnrmFrame();
  std::uint8_t output[4] = {};
  std::size_t writtenSize = 99u;

  EXPECT_EQ(DLMS_HDLC_STATUS_OUTPUT_BUFFER_TOO_SMALL,
            dlms_hdlc_encode_frame(&frame,
                                   0,
                                   output,
                                   sizeof(output),
                                   &writtenSize));
  EXPECT_EQ(9u, writtenSize);
  EXPECT_EQ(0u, output[0]);
  EXPECT_EQ(0u, output[1]);
}

TEST(HdlcCApi, ValidatesNullArguments)
{
  const dlms_hdlc_frame_t frame = MakeSnrmFrame();
  std::uint8_t output[32] = {};
  std::size_t writtenSize = 0u;

  EXPECT_EQ(DLMS_HDLC_STATUS_INVALID_ARGUMENT,
            dlms_hdlc_encode_frame(0, 0, output, sizeof(output), &writtenSize));
  EXPECT_EQ(DLMS_HDLC_STATUS_INVALID_ARGUMENT,
            dlms_hdlc_encode_frame(&frame, 0, 0, sizeof(output), &writtenSize));
  EXPECT_EQ(DLMS_HDLC_STATUS_INVALID_ARGUMENT,
            dlms_hdlc_encode_frame(&frame, 0, output, sizeof(output), 0));
  EXPECT_EQ(DLMS_HDLC_STATUS_INVALID_ARGUMENT,
            dlms_hdlc_decode_frame(0, 0, 0, 0, 0, 0, 0));
}

TEST(HdlcCApi, StreamDecoderRejectsNullInformationBufferForPayloadFrame)
{
  const std::uint8_t payload[] = {0xE6u, 0xE6u, 0x00u};
  const dlms_hdlc_frame_t frame = MakeInformationFrame(payload, sizeof(payload));
  std::uint8_t encoded[32] = {};
  std::size_t encodedSize = 0u;
  EncodeFrameOrFail(frame, encoded, sizeof(encoded), &encodedSize);

  dlms_hdlc_stream_decoder_t* decoder = 0;
  ASSERT_EQ(DLMS_HDLC_STATUS_OK,
            dlms_hdlc_stream_decoder_create(0, &decoder));

  dlms_hdlc_frame_t decoded;
  std::size_t informationSize = 99u;
  EXPECT_EQ(DLMS_HDLC_STATUS_INVALID_ARGUMENT,
            dlms_hdlc_stream_decoder_push(decoder,
                                          encoded,
                                          encodedSize,
                                          &decoded,
                                          0,
                                          sizeof(payload),
                                          &informationSize));
  EXPECT_EQ(0u, informationSize);

  dlms_hdlc_stream_decoder_destroy(decoder);
}

TEST(HdlcCApi, StreamDecoderResetsAfterDecodeError)
{
  const std::uint8_t malformed[] = {0x7Eu, 0x00u, 0x00u, 0x7Eu};
  dlms_hdlc_stream_decoder_t* decoder = 0;
  ASSERT_EQ(DLMS_HDLC_STATUS_OK,
            dlms_hdlc_stream_decoder_create(0, &decoder));

  dlms_hdlc_frame_t decoded;
  std::uint8_t information[8] = {};
  std::size_t informationSize = 99u;
  EXPECT_NE(DLMS_HDLC_STATUS_OK,
            dlms_hdlc_stream_decoder_push(decoder,
                                          malformed,
                                          sizeof(malformed),
                                          &decoded,
                                          information,
                                          sizeof(information),
                                          &informationSize));
  EXPECT_EQ(0u, informationSize);

  const dlms_hdlc_frame_t frame = MakeSnrmFrame();
  std::uint8_t encoded[32] = {};
  std::size_t encodedSize = 0u;
  EncodeFrameOrFail(frame, encoded, sizeof(encoded), &encodedSize);

  ASSERT_EQ(DLMS_HDLC_STATUS_OK,
            dlms_hdlc_stream_decoder_push(decoder,
                                          encoded,
                                          encodedSize,
                                          &decoded,
                                          information,
                                          sizeof(information),
                                          &informationSize));
  EXPECT_EQ(0u, informationSize);
  EXPECT_EQ(0x93u, decoded.control);

  dlms_hdlc_stream_decoder_destroy(decoder);
}

TEST(HdlcCApi, OutputFramesAreClearedBeforeValidationErrors)
{
  dlms_hdlc_frame_t decoded = FilledFrame();
  std::uint8_t information[8] = {};
  std::size_t informationSize = 99u;
  EXPECT_EQ(DLMS_HDLC_STATUS_INVALID_ARGUMENT,
            dlms_hdlc_decode_frame(0,
                                   1u,
                                   0,
                                   &decoded,
                                   information,
                                   sizeof(information),
                                   &informationSize));
  ExpectClearedFrame(decoded);
  EXPECT_EQ(0u, informationSize);

  dlms_hdlc_stream_decoder_t* decoder = 0;
  ASSERT_EQ(DLMS_HDLC_STATUS_OK,
            dlms_hdlc_stream_decoder_create(0, &decoder));
  decoded = FilledFrame();
  informationSize = 99u;
  EXPECT_EQ(DLMS_HDLC_STATUS_INVALID_ARGUMENT,
            dlms_hdlc_stream_decoder_push(decoder,
                                          0,
                                          1u,
                                          &decoded,
                                          information,
                                          sizeof(information),
                                          &informationSize));
  ExpectClearedFrame(decoded);
  EXPECT_EQ(0u, informationSize);
  dlms_hdlc_stream_decoder_destroy(decoder);

  dlms_hdlc_reassembler_t* reassembler = 0;
  ASSERT_EQ(DLMS_HDLC_STATUS_OK,
            dlms_hdlc_reassembler_create(0, &reassembler));
  dlms_hdlc_frame_t output = FilledFrame();
  std::size_t outputInformationSize = 99u;
  int hasCompletedFrame = 99;
  EXPECT_EQ(DLMS_HDLC_STATUS_INVALID_ARGUMENT,
            dlms_hdlc_reassembler_push_frame(reassembler,
                                             0,
                                             &output,
                                             information,
                                             sizeof(information),
                                             &outputInformationSize,
                                             &hasCompletedFrame));
  ExpectClearedFrame(output);
  EXPECT_EQ(0u, outputInformationSize);
  EXPECT_EQ(0, hasCompletedFrame);
  dlms_hdlc_reassembler_destroy(reassembler);
}

TEST(HdlcCApi, StreamDecoderLifecycle)
{
  dlms_hdlc_stream_decoder_t* decoder = 0;

  ASSERT_EQ(DLMS_HDLC_STATUS_OK,
            dlms_hdlc_stream_decoder_create(0, &decoder));
  ASSERT_NE(static_cast<dlms_hdlc_stream_decoder_t*>(0), decoder);
  dlms_hdlc_stream_decoder_reset(decoder);
  dlms_hdlc_stream_decoder_destroy(decoder);

  EXPECT_EQ(DLMS_HDLC_STATUS_INVALID_ARGUMENT,
            dlms_hdlc_stream_decoder_create(0, 0));
  dlms_hdlc_stream_decoder_reset(0);
  dlms_hdlc_stream_decoder_destroy(0);
}

TEST(HdlcCApi, ReassemblerLifecycle)
{
  dlms_hdlc_reassembler_t* reassembler = 0;

  ASSERT_EQ(DLMS_HDLC_STATUS_OK,
            dlms_hdlc_reassembler_create(0, &reassembler));
  ASSERT_NE(static_cast<dlms_hdlc_reassembler_t*>(0), reassembler);
  dlms_hdlc_reassembler_reset(reassembler);
  dlms_hdlc_reassembler_destroy(reassembler);

  EXPECT_EQ(DLMS_HDLC_STATUS_INVALID_ARGUMENT,
            dlms_hdlc_reassembler_create(0, 0));
  dlms_hdlc_reassembler_reset(0);
  dlms_hdlc_reassembler_destroy(0);
}

TEST(HdlcCApi, ReassemblerRejectsNullInformationBufferForPayloadFrame)
{
  const std::uint8_t payload[] = {0xE6u, 0xE6u, 0x00u};
  const dlms_hdlc_frame_t input = MakeInformationFrame(payload, sizeof(payload));
  dlms_hdlc_reassembler_t* reassembler = 0;
  ASSERT_EQ(DLMS_HDLC_STATUS_OK,
            dlms_hdlc_reassembler_create(0, &reassembler));

  dlms_hdlc_frame_t output;
  std::size_t outputInformationSize = 99u;
  int hasCompletedFrame = 99;
  EXPECT_EQ(DLMS_HDLC_STATUS_INVALID_ARGUMENT,
            dlms_hdlc_reassembler_push_frame(reassembler,
                                             &input,
                                             &output,
                                             0,
                                             sizeof(payload),
                                             &outputInformationSize,
                                             &hasCompletedFrame));
  EXPECT_EQ(0u, outputInformationSize);
  EXPECT_EQ(0, hasCompletedFrame);

  dlms_hdlc_reassembler_destroy(reassembler);
}

TEST(HdlcCApi, InvalidInputsDoNotCrash)
{
  dlms_hdlc_frame_t frame = MakeSnrmFrame();
  frame.destination_address_size = 3u;
  std::uint8_t output[32] = {};
  std::size_t writtenSize = 0u;

  EXPECT_EQ(DLMS_HDLC_STATUS_UNSUPPORTED_ADDRESS,
            dlms_hdlc_encode_frame(&frame,
                                   0,
                                   output,
                                   sizeof(output),
                                   &writtenSize));
}

} // namespace
