# 00. LLC Requirements

## 1. Scope

The library implements the DLMS/COSEM LLC sub-layer codec used by the
3-layer, connection-oriented HDLC based communication profile.

The LLC layer is used as a protocol selector between the APDU layer and the
HDLC/MAC layer. It adds or removes a 3-byte LLC header around opaque APDU bytes.

## 2. Normative Assumptions

The implementation targets IEC 62056-46 HDLC-based LLC behavior.

The LLC header consists of:

```text
Destination LSAP
Source LSAP
Control
```

For normal DLMS/COSEM traffic:

```text
client -> server: E6 E6 00 || APDU
server -> client: E6 E7 00 || APDU
```

The control byte is currently fixed to `0x00`.

The Destination LSAP `0xFF` is reserved for broadcast receive handling. The
codec may accept it on decode when the caller explicitly enables broadcast
destination handling. Normal encode APIs must not emit `0xFF`.

## 3. Included in v1

- LPDU encode/decode.
- LLC header encode/decode.
- Client request helper.
- Server response helper.
- Header validation.
- Broadcast Destination LSAP decode policy.
- Caller-provided-buffer API.
- Convenience vector API.
- Stable C ABI wrapper.
- GoogleTest unit tests.
- Root integration tests with `dlms-hdlc`.
- Doxygen documentation for public API.

## 4. Excluded from v1

- APDU parsing.
- HDLC frame codec.
- HDLC session layer.
- HDLC segmentation/reassembly implementation.
- Transport I/O.
- WRAPPER profile.
- IEC 61334-4-32 connectionless LLC profile.
- DL-Reply and DL-Update-Reply services.
- Security and ciphering.

## 5. Error Handling

Public/runtime API paths must not throw exceptions.

Errors are returned as status codes. Runtime paths must not call `abort` or rely
on assertions for input validation.

## 6. Buffer Policy

The primary API uses caller-provided buffers and must not allocate memory.

Convenience APIs may use `std::vector`. Allocation failures must be translated
to `InternalError`.

## 7. Segmentation Boundary

The LLC codec does not segment data.

The HDLC session/segmentation layer must deliver a complete LLC LPDU to the LLC
decoder. HDLC continuation fragments without an LLC header are not valid inputs
for complete LPDU decoding.

## 8. Compatibility Requirement

The codec must preserve APDU bytes exactly. It must not inspect, normalize, or
rewrite LSDU contents.
