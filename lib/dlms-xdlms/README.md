# dlms-xdlms

`dlms-xdlms` provides high-level xDLMS service orchestration over an already
established DLMS/COSEM association. The component owns client request flows and
server-side GET/SET/ACTION dispatch contracts.

Implemented scope includes:

- GET, SET, and ACTION request/response orchestration;
- server-side GET, SET, and ACTION dispatch boundaries;
- service-specific block transfer helpers;
- optional APDU security processor integration;
- operation over `dlms::profile::IApduChannel` and abstract association state.

The component does not own transport I/O, profile framing, association opening,
COSEM object storage, public client facades, or endpoint lifecycle.

## Build

```text
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

## Documentation

- [Requirements](docs/00_xdlms_requirements.md)
- [API](docs/01_xdlms_api.md)
- [Test Plan](docs/03_xdlms_test_plan.md)
- [Architecture](docs/architecture.md)
- [Implementation Plan](docs/04_xdlms_implementation_plan.md)
