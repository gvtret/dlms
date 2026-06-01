# DLMS/COSEM Framework

This repository contains a C++11 DLMS/COSEM framework as a single
monorepository. Component boundaries remain explicit under `lib/`, while the
root repository owns versioning, integration tests, packaging, and release
artifacts.

## Components

| Component | Responsibility |
|---|---|
| `lib/dlms-hdlc` | HDLC frame codec and session state machine |
| `lib/dlms-llc` | LLC LPDU codec |
| `lib/dlms-wrapper` | DLMS Wrapper WPDU codec |
| `lib/dlms-apdu` | ACSE and xDLMS APDU codecs |
| `lib/dlms-transport` | TCP, UDP, serial, timers, tracing, and transport APIs |
| `lib/dlms-profile` | Wrapper and HDLC APDU channels |
| `lib/dlms-association` | DLMS/COSEM association open/release state machines |
| `lib/dlms-security` | HLS and Suite 0 AES-GCM security helpers |
| `lib/dlms-xdlms` | GET, SET, ACTION, block-transfer service orchestration |
| `lib/dlms-cosem` | COSEM logical-device and object model |
| `lib/dlms-server` | Server-side xDLMS dispatch over COSEM objects |
| `lib/dlms-client` | Public client facade |
| `lib/dlms-endpoint` | Runtime composition for client, server, push listener, and gateway |

## Build And Test

The default root build includes deterministic tests and examples. Live meter
checks are opt-in.

On MSYS2 MinGW:

```sh
cd /e/work/dlms
mkdir -p build-mingw64/tmp
export PATH=/mingw64/bin:/usr/bin:$PATH
export TMP=/e/work/dlms/build-mingw64/tmp
export TEMP=/e/work/dlms/build-mingw64/tmp
export TMPDIR=/e/work/dlms/build-mingw64/tmp

cmake -S . -B build-mingw64 -G Ninja
cmake --build build-mingw64
ctest --test-dir build-mingw64 --output-on-failure
```

## Install

The root build exports a CMake package named `DLMSFramework`:

```sh
cmake --install build-mingw64 --prefix /tmp/dlms-install
```

Consumer projects can then use:

```cmake
find_package(DLMSFramework REQUIRED CONFIG)
target_link_libraries(app PRIVATE dlms::framework)
```

Aggregate targets are available for narrower linking:

| Target | Scope |
|---|---|
| `dlms::codec` | HDLC, LLC, Wrapper, APDU codecs |
| `dlms::io` | transport and profile channels |
| `dlms::protocol` | association, security, xDLMS |
| `dlms::cosem_server` | COSEM object model and server dispatch |
| `dlms::runtime` | client and endpoint runtime composition |
| `dlms::framework` | all framework targets |

## Versioning

The canonical framework version is stored in `VERSION` and follows SemVer.
See `docs/versioning.md` for the compatibility policy.

## Documentation

- `docs/system_architecture.md` describes the full layered architecture.
- `docs/architecture_consolidation.md` documents the monorepo and package
  consolidation decision.
- Each component keeps its own requirements, API, architecture, and test-plan
  documents under `lib/<component>/docs`.
