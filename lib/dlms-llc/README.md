# DLMS/COSEM LLC Codec

Portable C++11 Logical Link Control sub-layer codec for DLMS/COSEM.

This repository contains the LLC codec for a DLMS/COSEM framework. The framework
is expected to include HDLC, LLC, WRAPPER and APDU codecs, while this repository
focuses on the LLC layer used by the 3-layer, connection-oriented HDLC based
communication profile.

## Scope

Version 1 implements the **LLC codec layer**.

Included:

- DLMS/COSEM LLC LPDU encoding and decoding
- client and server LPDU support
- LLC header encoding and decoding
- Destination LSAP, Source LSAP and control byte validation
- client request helper for `E6 E6 00`
- server response helper for `E6 E7 00`
- broadcast Destination LSAP decode policy
- status-code based error handling
- no exceptions in public/runtime API paths
- stable C ABI wrapper
- Doxygen-documented public API
- CMake 3.16+ build system
- GoogleTest-based test suite
- root integration tests with `dlms-hdlc`

Not included in v1:

- APDU codec
- APDU parsing
- HDLC codec
- HDLC session layer
- HDLC segmentation and reassembly implementation
- transport layer
- WRAPPER codec
- IEC 61334-4-32 connectionless LLC profile
- DL-Reply and DL-Update-Reply services
- security and ciphering

## Target Architecture

Expected future HDLC-based DLMS/COSEM stack:

```text
+-----------------------------+
| APDU codec                  |
+-----------------------------+
| LLC codec                   |
+-----------------------------+
| HDLC session layer          |
+-----------------------------+
| HDLC codec                  |
+-----------------------------+
| Transport: UART/TCP/etc.    |
+-----------------------------+
```

Expected future WRAPPER-based stack:

```text
+-----------------------------+
| APDU codec                  |
+-----------------------------+
| WRAPPER codec               |
+-----------------------------+
| Transport: TCP/UDP          |
+-----------------------------+
```

The LLC codec does **not** parse APDUs and does **not** encode HDLC frames. It
treats the LSDU as opaque bytes. In DLMS/COSEM HDLC profile, the LLC LPDU is
carried by the HDLC `Information` field.

## Key Design Decisions

| Area | Decision |
|---|---|
| Language | C++11 |
| Build system | CMake 3.16+ |
| Error handling | status codes |
| Exceptions | not used in public/runtime API paths |
| Target roles | client and server |
| LLC profile | IEC 62056-46 HDLC-based LLC |
| LPDU header | 3 bytes: DSAP, SSAP, control |
| Client request header | `E6 E6 00` |
| Server response header | `E6 E7 00` |
| Control byte | `0x00` in v1 |
| Broadcast DSAP | accepted only by explicit decode policy |
| Segmentation | boundary documented, not implemented here |
| C ABI | separate stable wrapper |
| Tests | GoogleTest |

## LPDU Format

Supported LPDU layout:

```text
Destination LSAP
Source LSAP
Control
LSDU
```

The first three bytes form the LLC header. The remaining bytes are the LSDU and
are treated as opaque APDU bytes.

Common DLMS/COSEM HDLC-based headers:

```text
client -> server: E6 E6 00 || APDU
server -> client: E6 E7 00 || APDU
```

The Destination LSAP `0xFF` is reserved for broadcast receive handling. Normal
encoding must not emit broadcast Destination LSAP unless a future API explicitly
adds such a mode.

## Limits

### `maximum_lpdu_size`

`maximum_lpdu_size` is the complete LLC LPDU size in bytes, including the
3-byte LLC header.

Default:

```text
maximum_lpdu_size = 65535
```

### `maximum_lsdu_size`

`maximum_lsdu_size` is the maximum number of bytes in the LLC LSDU.

Formula:

```text
maximum_lsdu_size = maximum_lpdu_size - 3 bytes LLC header
```

LLC bytes are part of the HDLC `Information` field and must not be counted as
HDLC header overhead.

### HDLC segmentation boundary

The LLC codec does not segment data. The HDLC session/segmentation layer must
deliver a complete LLC LPDU to the LLC decoder.

For HDLC fragments:

```text
I-COMPLETE:
  Information contains full LLC LPDU

I-FIRST-FRAGMENT:
  Information starts with LLC header followed by first LSDU bytes

I-FRAGMENT:
  Information contains continuation bytes without LLC header

I-LAST-FRAGMENT:
  Information contains final continuation bytes without LLC header
```

## Repository Layout

Current layout:

```text
.
├── CMakeLists.txt
├── include/
│   └── dlms/
│       └── llc/
│           ├── llc_types.hpp
│           ├── llc_error.hpp
│           ├── llc_header.hpp
│           ├── llc_codec.hpp
│           └── llc_c_api.h
├── src/
│   └── llc/
│       ├── llc_header.cpp
│       ├── llc_codec.cpp
│       └── llc_c_api.cpp
├── test/
│   ├── CMakeLists.txt
│   └── llc/
│       ├── test_llc_header.cpp
│       ├── test_llc_codec.cpp
│       ├── test_llc_c_api.cpp
│       ├── test_llc_c_header.c
│       └── test_llc_vectors.cpp
└── docs/
    ├── 00_llc_requirements.md
    ├── 01_llc_codec_api.md
    ├── 02_llc_c_api.md
    ├── 03_llc_segmentation_boundary.md
    └── 04_llc_test_plan.md
```

Root integration tests live in the parent workspace:

```text
E:/work/dlms/test/integration/
```

## Build

Configure:

```bash
cmake -S . -B build -DDLMS_BUILD_TESTS=ON
```

Build:

```bash
cmake --build build
```

Run tests:

```bash
ctest --test-dir build --output-on-failure
```

For this workspace, use the MSYS2 MinGW64 environment:

```powershell
C:\msys\usr\bin\bash.exe -lc 'export PATH=/mingw64/bin:/usr/bin:$PATH; cmake -S /e/work/dlms/lib/dlms-llc -B /e/work/dlms/lib/dlms-llc/build -G "MinGW Makefiles"'
```

## CMake Options

CMake options:

```text
DLMS_BUILD_TESTS       Build GoogleTest tests
DLMS_BUILD_C_API       Build stable C ABI wrapper
DLMS_USE_SYSTEM_GTEST  Use system-installed GoogleTest
```

Example:

```bash
cmake -S . -B build \
  -DDLMS_BUILD_TESTS=ON \
  -DDLMS_BUILD_C_API=ON \
  -DDLMS_USE_SYSTEM_GTEST=OFF
```

## Error Handling

The library uses status codes only.

Example status model:

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

No public/runtime API path should throw exceptions.

## C++ API Direction

The library provides two API styles.

Strict no-allocation API:

```cpp
LlcStatus EncodeLpduToBuffer(
  const LlcHeader& header,
  const std::uint8_t* lsdu,
  std::size_t lsduSize,
  std::uint8_t* output,
  std::size_t outputSize,
  std::size_t& writtenSize);
```

Convenience API:

```cpp
LlcStatus EncodeLpdu(
  const LlcHeader& header,
  const std::uint8_t* lsdu,
  std::size_t lsduSize,
  std::vector<std::uint8_t>& output);
```

The no-allocation API is the primary API. STL-based APIs are convenience wrappers.

## C ABI Direction

The C ABI is a separate stable wrapper over the C++ implementation.

Rules:

- `extern "C"`
- no C++ types in public C headers
- fixed-width integer types
- caller-provided buffers
- status codes only
- explicit status-name mapping, independent from C++ enum declaration order

Example:

```c
dlms_llc_status_t dlms_llc_encode_request(
  const uint8_t* apdu,
  size_t apdu_size,
  uint8_t* output,
  size_t output_size,
  size_t* written_size);
```

## Segmentation

The codec does not implement segmentation.

The LLC/HDLC boundary rules are:

```text
LLC encoder:
  builds one complete LPDU from one LSDU

HDLC segmentation:
  may split the complete LPDU across HDLC I-frames

LLC decoder:
  expects one complete LPDU after HDLC reassembly
```

The LLC decoder must not treat HDLC continuation fragments as complete LPDUs.
It must return `NeedMoreData` or a header validation error when fewer than three
LLC header bytes are available.

## HDLC Integration

The LLC codec is designed to be used with `dlms-hdlc`.

Expected flow:

```text
APDU bytes
-> LLC EncodeLpdu
-> HDLC Information field
-> HDLC encode/session/transport
```

Receive flow:

```text
HDLC decode/reassembly
-> complete HDLC Information field
-> LLC DecodeLpdu
-> APDU bytes
```

The LLC codec does not own HDLC addresses, HDLC control fields, sequence numbers,
SNRM/UA, DISC/UA, RR, timeouts, retransmission, or transport I/O.

## Test Plan

The test suite must cover:

- LLC header construction
- DSAP validation
- SSAP validation
- control byte validation
- LPDU encoding and decoding
- empty LSDU
- output buffer too small
- short LPDU input
- broadcast Destination LSAP decode policy
- C ABI
- real DLMS/COSEM LLC vectors
- integration with HDLC Information field

Required real vectors:

- client request LPDU carrying AARQ or xDLMS APDU
- server response LPDU carrying AARE or xDLMS APDU
- HDLC I-frame carrying LLC payload
- invalid LPDU with unsupported LLC control byte

## Development Phases

### Phase 0

Documentation and requirements.

Current status:

```text
Completed
```

Documents:

```text
llc_codec_plan.md
docs/00_llc_requirements.md
docs/01_llc_codec_api.md
docs/02_llc_c_api.md
docs/03_llc_segmentation_boundary.md
docs/04_llc_test_plan.md
```

### Phase 1

Initial project structure:

- CMake root project
- include tree
- source tree
- test tree
- empty buildable library
- empty test executable

### Phase 2

Status/error model.

### Phase 3

LLC constants and header validation.

### Phase 4

LPDU encoder.

### Phase 5

LPDU decoder.

### Phase 6

Stable C ABI.

### Phase 7

Root integration tests with `dlms-hdlc`.

### Phase 8

Doxygen public API documentation.

## License

Not selected yet.

Before the first public release, choose and add a license file, for example:

- MIT
- Apache-2.0
- BSD-2-Clause
- BSD-3-Clause
