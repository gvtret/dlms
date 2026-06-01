# 01. LLC Codec API

## 1. Design Goals

The C++ API must be small, explicit, and usable in constrained environments.

Requirements:

- C++11.
- Status-code based errors.
- No exceptions in public/runtime API paths.
- Strict no-allocation API as the primary API.
- Convenience vector API as wrappers.
- APDU/LSDU bytes treated as opaque data.

## 2. Constants

```cpp
static const std::uint8_t kLlcDsapDlms = 0xE6;
static const std::uint8_t kLlcSsapClient = 0xE6;
static const std::uint8_t kLlcSsapServer = 0xE7;
static const std::uint8_t kLlcDsapBroadcast = 0xFF;
static const std::uint8_t kLlcControlDefault = 0x00;
static const std::size_t kLlcHeaderSize = 3;
```

## 3. Status Model

```cpp
enum class LlcStatus
{
  Ok,

  NeedMoreData,
  OutputBufferTooSmall,

  InvalidArgument,
  InvalidHeader,
  InvalidDsap,
  InvalidSsap,
  InvalidControl,
  InvalidLpduLength,

  LsduTooLarge,
  BroadcastEncodeForbidden,

  UnsupportedAddress,
  UnsupportedControl,
  UnsupportedFeature,

  InternalError
};
```

`LlcStatus` is a named C++ status model. Its numeric values and declaration
order are not part of the C++ API contract and must not be tested directly.

## 4. Header Model

```cpp
enum class LlcDirection
{
  ClientToServer,
  ServerToClient
};

struct LlcHeader
{
  std::uint8_t dsap;
  std::uint8_t ssap;
  std::uint8_t control;
};
```

## 5. LPDU Model

Non-owning LPDU view:

```cpp
struct LlcLpdu
{
  LlcHeader header;
  const std::uint8_t* lsduData;
  std::size_t lsduSize;
};
```

Owning LPDU container:

```cpp
struct LlcLpduBuffer
{
  LlcHeader header;
  std::vector<std::uint8_t> lsdu;
};
```

## 6. Header Helpers

```cpp
LlcHeader MakeLlcHeader(LlcDirection direction);

bool IsKnownDlmsLlcHeader(const LlcHeader& header);

LlcStatus ValidateLlcHeader(
  const LlcHeader& header,
  bool allowBroadcastDestination);
```

Validation rules:

- `control` must be `0x00` in v1.
- `ssap` must be a known DLMS/COSEM LLC SAP.
- `dsap` must be `0xE6`, or `0xFF` only when broadcast decode is explicitly
  allowed.

## 7. Strict API

```cpp
LlcStatus EncodeLpduToBuffer(
  const LlcHeader& header,
  const std::uint8_t* lsdu,
  std::size_t lsduSize,
  std::uint8_t* output,
  std::size_t outputSize,
  std::size_t& writtenSize);
```

```cpp
LlcStatus DecodeLpduFromBuffer(
  const std::uint8_t* input,
  std::size_t inputSize,
  bool allowBroadcastDestination,
  LlcLpdu& lpdu);
```

`DecodeLpduFromBuffer` returns a view into `input`. The caller must keep `input`
alive while using the returned `LlcLpdu`.

## 8. Convenience API

```cpp
LlcStatus EncodeLpdu(
  const LlcHeader& header,
  const std::uint8_t* lsdu,
  std::size_t lsduSize,
  std::vector<std::uint8_t>& output);
```

```cpp
LlcStatus DecodeLpdu(
  const std::uint8_t* input,
  std::size_t inputSize,
  bool allowBroadcastDestination,
  LlcLpduBuffer& lpdu);
```

## 9. DLMS Helpers

```cpp
LlcStatus EncodeDlmsRequest(
  const std::uint8_t* apdu,
  std::size_t apduSize,
  std::vector<std::uint8_t>& output);
```

```cpp
LlcStatus EncodeDlmsResponse(
  const std::uint8_t* apdu,
  std::size_t apduSize,
  std::vector<std::uint8_t>& output);
```

These helpers are convenience wrappers over `EncodeLpdu`.
