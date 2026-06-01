# 04. LLC Test Plan

## 1. Test Framework

GoogleTest is mandatory.

The CMake project must support:

```text
DLMS_BUILD_TESTS=ON
DLMS_USE_SYSTEM_GTEST=ON/OFF
```

## 2. Header Tests

Required tests:

- make client-to-server header returns `E6 E6 00`.
- make server-to-client header returns `E6 E7 00`.
- validate client request header.
- validate server response header.
- reject unknown Destination LSAP.
- reject unknown Source LSAP.
- reject unsupported control byte.
- accept broadcast Destination LSAP only when decode policy allows it.
- reject broadcast Destination LSAP for normal encode.

## 3. LPDU Encode Tests

Required tests:

- encode client request.
- encode server response.
- encode empty LSDU.
- encode APDU bytes unchanged.
- encode payload containing `0x7E`.
- output buffer too small.
- null pointer validation.
- LSDU too large.

## 4. LPDU Decode Tests

Required tests:

- decode client request.
- decode server response.
- decode empty LSDU.
- decode returns correct LSDU view.
- decode does not modify APDU bytes.
- reject empty input.
- reject one-byte input.
- reject two-byte input.
- reject invalid DSAP.
- reject invalid SSAP.
- reject invalid control byte.
- broadcast Destination LSAP policy.

## 5. C ABI Tests

Required tests:

- C header compiles as C.
- encode request through C API.
- encode response through C API.
- encode generic LPDU through C API.
- decode LPDU through C API.
- validate header through C API.
- output buffer too small.
- null argument validation.
- no crash on invalid inputs.

## 6. Real DLMS/COSEM Vectors

The test suite must include real or trace-backed vectors for:

- client request LPDU carrying AARQ or xDLMS APDU.
- server response LPDU carrying AARE or xDLMS APDU.
- HDLC I-frame carrying LLC payload.
- invalid LPDU with unsupported LLC control byte.

Known vectors must be stored in a dedicated test file.

## 7. Root Integration Tests

Integration tests in `E:/work/dlms/test/integration` must cover:

- client request LPDU survives HDLC encode/decode roundtrip.
- server response LPDU survives HDLC encode/decode roundtrip.
- historical DLMS HDLC vector decodes LLC header.
- HDLC Information field without a full LLC header is rejected by LLC.
- unsupported LLC control is rejected after valid HDLC decode.
- APDU byte `0x7E` survives LLC and HDLC roundtrip.

## 8. Documentation Checks

Public API headers must document:

- ownership of input and output buffers.
- output size behavior.
- status code meanings.
- broadcast Destination LSAP policy.
- segmentation boundary.
