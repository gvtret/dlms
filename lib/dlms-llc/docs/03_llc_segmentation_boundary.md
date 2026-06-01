# 03. LLC Segmentation Boundary

## 1. Scope

The LLC codec does not implement segmentation or reassembly.

This document records the boundary between LLC LPDU handling and HDLC
segmentation in the DLMS/COSEM HDLC-based profile.

## 2. Layer Ownership

LLC owns:

- DSAP.
- SSAP.
- LLC control byte.
- LPDU encode/decode.
- LSDU byte preservation.

HDLC/session owns:

- HDLC frame format.
- HDLC addresses.
- HDLC control field.
- sequence numbers.
- SNRM/UA and DISC/UA.
- RR/RNR.
- segmentation and reassembly.
- retransmission and timeout policy.

## 3. Complete LPDU Rule

The LLC decoder expects one complete LPDU:

```text
DSAP SSAP CONTROL LSDU...
```

If fewer than three bytes are available, the decoder returns `NeedMoreData`.

The decoder must not guess missing LLC headers for continuation fragments.

## 4. HDLC Frame Cases

For HDLC I-frame transfer:

```text
I-COMPLETE:
  HDLC Information field contains full LLC LPDU

I-FIRST-FRAGMENT:
  HDLC Information field starts with LLC header followed by first LSDU bytes

I-FRAGMENT:
  HDLC Information field contains continuation bytes without LLC header

I-LAST-FRAGMENT:
  HDLC Information field contains final continuation bytes without LLC header
```

## 5. Recommended Receive Flow

```text
HDLC stream decode
-> HDLC frame decode
-> HDLC session validation
-> HDLC reassembly when segmented
-> complete HDLC Information field
-> LLC DecodeLpdu
-> APDU bytes
```

## 6. Recommended Send Flow

```text
APDU bytes
-> LLC EncodeLpdu
-> complete LLC LPDU
-> HDLC segmentation if needed
-> HDLC session/frame encode
-> transport
```

## 7. Negative Cases

The LLC decoder must reject or report incomplete input for:

- empty input.
- one-byte or two-byte input.
- continuation fragment without LLC header.
- unsupported DSAP.
- unsupported SSAP.
- unsupported control byte.

## 8. Integration Test Requirement

Root integration tests must cover:

- LLC request LPDU survives HDLC encode/decode.
- LLC response LPDU survives HDLC encode/decode.
- real HDLC I-frame carrying LLC payload decodes to APDU bytes.
- short HDLC Information field is not accepted as a complete LPDU.
- unsupported LLC control byte is rejected after valid HDLC decode.
