# dlms-client

`dlms-client` is the public client facade layer for the DLMS/COSEM framework.

It composes lower components into an application-facing synchronous API:

- `dlms-transport` for TCP, UDP, serial, and future transport construction;
- `dlms-profile` for Wrapper and HDLC APDU channels;
- `dlms-association` for application association lifecycle;
- `dlms-xdlms` for GET, SET, and ACTION service primitives.
- `dlms-security` for optional HLS and ciphered APDU processing.

The facade must not implement protocol codecs, association negotiation rules,
COSEM object storage, or xDLMS APDU semantics. Those responsibilities stay in
their dedicated components.

Applications can use the options-based constructor for default composition, or
inject lower-layer pieces explicitly. `IClientXdlmsService` is the abstract
GET/SET/ACTION backend port for callers that need a custom xDLMS service
implementation while keeping the `DlmsClient` facade API.

GUI clients can use `ReadAttribute`, `WriteAttribute`, and `CallMethod` when
the UI works with class id, OBIS logical name, and attribute/method id directly.
These helpers still pass complete encoded DLMS `Data` values across the facade,
but they return detailed result structs with invoke id and access/action result
bytes for diagnostics and user-visible error reporting.

`client_data.hpp` provides typed encode/decode helpers for common DLMS `Data`
values such as boolean, signed/unsigned scalars, enum, and octet-string. These
helpers are intended for UI models that need typed values while the transport
facade keeps exchanging complete encoded DLMS `Data` bytes.

The root `tools/client_gui_read_example.cpp` program shows the intended GUI
backend pattern: use `ReadAttribute` with class id and OBIS logical name, check
`accessResult`, and decode the returned encoded DLMS `Data` through
`client_data.hpp`.

## Documentation

- [Requirements](docs/00_client_requirements.md)
- [API](docs/01_client_api.md)
- [Architecture](docs/02_client_architecture.md)
- [Test Plan](docs/03_client_test_plan.md)
- [Implementation Plan](docs/04_client_implementation_plan.md)
