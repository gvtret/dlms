# 02. LLC C API

## 1. Goals

The C ABI is a stable wrapper over the C++ implementation.

Rules:

- `extern "C"`.
- No C++ types in public C headers.
- Fixed-width integer types.
- Caller-provided buffers.
- Status codes only.
- No exceptions crossing the C ABI.
- The C ABI maps status names explicitly and must not depend on the C++
  `LlcStatus` declaration order.

## 2. Header

The C ABI header is:

```text
include/dlms/llc/llc_c_api.h
```

The implementation file is:

```text
src/llc/llc_c_api.cpp
```

## 3. Status Enum

```c
typedef enum dlms_llc_status_t
{
  DLMS_LLC_STATUS_OK,
  DLMS_LLC_STATUS_NEED_MORE_DATA,
  DLMS_LLC_STATUS_OUTPUT_BUFFER_TOO_SMALL,
  DLMS_LLC_STATUS_INVALID_ARGUMENT,
  DLMS_LLC_STATUS_INVALID_HEADER,
  DLMS_LLC_STATUS_INVALID_DSAP,
  DLMS_LLC_STATUS_INVALID_SSAP,
  DLMS_LLC_STATUS_INVALID_CONTROL,
  DLMS_LLC_STATUS_INVALID_LPDU_LENGTH,
  DLMS_LLC_STATUS_LSDU_TOO_LARGE,
  DLMS_LLC_STATUS_BROADCAST_ENCODE_FORBIDDEN,
  DLMS_LLC_STATUS_UNSUPPORTED_ADDRESS,
  DLMS_LLC_STATUS_UNSUPPORTED_CONTROL,
  DLMS_LLC_STATUS_UNSUPPORTED_FEATURE,
  DLMS_LLC_STATUS_INTERNAL_ERROR
} dlms_llc_status_t;
```

Tests should verify that C API functions return the expected named status for
each behavior. They must not mirror or assert the declaration order of the C++
`LlcStatus` enum.

## 4. C Structs

```c
typedef struct dlms_llc_header_t
{
  uint8_t dsap;
  uint8_t ssap;
  uint8_t control;
} dlms_llc_header_t;
```

```c
typedef struct dlms_llc_lpdu_view_t
{
  dlms_llc_header_t header;
  const uint8_t* lsdu;
  size_t lsdu_size;
} dlms_llc_lpdu_view_t;
```

The `lsdu` pointer in `dlms_llc_lpdu_view_t` points into the caller-provided
input buffer.

## 5. Encode API

```c
dlms_llc_status_t dlms_llc_encode_lpdu(
  const dlms_llc_header_t* header,
  const uint8_t* lsdu,
  size_t lsdu_size,
  uint8_t* output,
  size_t output_size,
  size_t* written_size);
```

```c
dlms_llc_status_t dlms_llc_encode_request(
  const uint8_t* apdu,
  size_t apdu_size,
  uint8_t* output,
  size_t output_size,
  size_t* written_size);
```

```c
dlms_llc_status_t dlms_llc_encode_response(
  const uint8_t* apdu,
  size_t apdu_size,
  uint8_t* output,
  size_t output_size,
  size_t* written_size);
```

## 6. Decode API

```c
dlms_llc_status_t dlms_llc_decode_lpdu(
  const uint8_t* input,
  size_t input_size,
  int allow_broadcast_destination,
  dlms_llc_lpdu_view_t* lpdu);
```

## 7. Validation API

```c
dlms_llc_status_t dlms_llc_validate_header(
  const dlms_llc_header_t* header,
  int allow_broadcast_destination);
```

## 8. Null Argument Policy

Functions must return `DLMS_LLC_STATUS_INVALID_ARGUMENT` for invalid pointers.

For zero-length LSDU:

- `lsdu == NULL` is allowed when `lsdu_size == 0`.
- `lsdu == NULL` is invalid when `lsdu_size > 0`.

## 9. Header Compile Test

The C header must compile as C and C++.

The test suite must include a `.c` translation unit that includes
`dlms/llc/llc_c_api.h`.
