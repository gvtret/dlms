# DLMS Framework — Package Consumer Minimum Includes

This document gives the **minimum CMake snippet and `#include` set** for every
public aggregate target exported by `DLMSFramework`. Each snippet is verified
by [`dlms_package_install_smoke`](../cmake/PackageInstallSmoke.cmake) and the
runnable consumer examples under
[`examples/package-consumers/`](../examples/package-consumers/).

If you only need a subset of the framework, link the most specific aggregate
target — that pulls in only the transitive component libraries it documents
and nothing else.

## Aggregate target dependency graph

```
dlms::framework
├── dlms::codec         → dlms_hdlc + dlms_llc + dlms_wrapper + dlms_apdu
├── dlms::io            → dlms_transport + dlms_profile
├── dlms::protocol      → dlms_association + dlms_security + dlms_xdlms
├── dlms::cosem_server  → dlms_cosem + dlms_server
└── dlms::runtime       → dlms_client + dlms_endpoint
```

`dlms::framework` is the convenience meta-target that pulls in every
component library; the five sub-aggregates exist so that consumers who only
need framing, only need the protocol layer, or only need the runtime facade
can avoid linking everything else.

OpenSSL is a transitive runtime dependency of `dlms::security` (and therefore
of `dlms::protocol`, `dlms::cosem_server`, `dlms::runtime`, `dlms::framework`).
`dlms::codec` and `dlms::io` have no OpenSSL dependency; the install smoke
verifies this by configuring those consumer builds with
`-DCMAKE_DISABLE_FIND_PACKAGE_OpenSSL=TRUE`.

## `dlms::codec` — DLMS framing (HDLC, LLC, Wrapper, APDU)

Pulls in: `dlms_hdlc`, `dlms_llc`, `dlms_wrapper`, `dlms_apdu`.
Use when you only need on-the-wire byte framing/parsing without any
association, ciphering or runtime.

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_dlms_codec_consumer LANGUAGES CXX)

find_package(DLMSFramework REQUIRED CONFIG COMPONENTS codec)

add_executable(my_dlms_codec_consumer main.cpp)
target_compile_features(my_dlms_codec_consumer PRIVATE cxx_std_11)
target_link_libraries(my_dlms_codec_consumer PRIVATE dlms::codec)
```

Minimum compilable consumer:

```cpp
#include "dlms/apdu/apdu_types.hpp"
#include "dlms/hdlc/hdlc_address.hpp"
#include "dlms/llc/llc_header.hpp"

int main() {
  const dlms::llc::LlcHeader header =
    dlms::llc::MakeLlcHeader(dlms::llc::LlcDirection::ClientToServer);
  return dlms::llc::IsKnownDlmsLlcHeader(header) ? 0 : 1;
}
```

Full runnable example: [`examples/package-consumers/codec/`](../examples/package-consumers/codec/).

## `dlms::io` — Transport and profile channels (Wrapper/TCP, HDLC profile)

Pulls in: `dlms_transport`, `dlms_profile`.
Use when you implement your own xDLMS layer or just need an `IApduChannel`
implementation over a stream/datagram transport.

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_dlms_io_consumer LANGUAGES CXX)

find_package(DLMSFramework REQUIRED CONFIG COMPONENTS io)

add_executable(my_dlms_io_consumer main.cpp)
target_compile_features(my_dlms_io_consumer PRIVATE cxx_std_11)
target_link_libraries(my_dlms_io_consumer PRIVATE dlms::io)
```

Minimum compilable consumer:

```cpp
#include "dlms/profile/profile_types.hpp"
#include "dlms/transport/transport_status.hpp"

int main() {
  const dlms::profile::ApduChannelOptions opts =
    dlms::profile::DefaultApduChannelOptions();
  return opts.maximumApduSize > 0 ? 0 : 1;
}
```

Full runnable example: [`examples/package-consumers/io/`](../examples/package-consumers/io/).

## `dlms::protocol` — Association, security, xDLMS

Pulls in: `dlms_association`, `dlms_security`, `dlms_xdlms`.
Use when you build a custom client or server on top of your own framing and
need association lifecycle, ciphering and xDLMS GET/SET/ACTION primitives.
Brings in the OpenSSL dependency.

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_dlms_protocol_consumer LANGUAGES CXX)

find_package(DLMSFramework REQUIRED CONFIG COMPONENTS protocol)

add_executable(my_dlms_protocol_consumer main.cpp)
target_compile_features(my_dlms_protocol_consumer PRIVATE cxx_std_11)
target_link_libraries(my_dlms_protocol_consumer PRIVATE dlms::protocol)
```

Minimum compilable consumer:

```cpp
#include "dlms/association/association_client_interface.hpp"
#include "dlms/security/key_store.hpp"
#include "dlms/xdlms/xdlms_types.hpp"

int main() {
  const dlms::xdlms::ServiceOptions opts =
    dlms::xdlms::DefaultServiceOptions();
  return opts.confirmed && opts.allowBlockTransfer ? 0 : 1;
}
```

Full runnable example: [`examples/package-consumers/protocol/`](../examples/package-consumers/protocol/).

## `dlms::cosem_server` — COSEM object model + server-side dispatch

Pulls in: `dlms_cosem`, `dlms_server`.
Use when you implement the server side of an association: the COSEM object
registry, logical device, server-side service dispatcher and server-side
xDLMS adapter.

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_dlms_cosem_server_consumer LANGUAGES CXX)

find_package(DLMSFramework REQUIRED CONFIG COMPONENTS cosem_server)

add_executable(my_dlms_cosem_server_consumer main.cpp)
target_compile_features(my_dlms_cosem_server_consumer PRIVATE cxx_std_11)
target_link_libraries(my_dlms_cosem_server_consumer PRIVATE dlms::cosem_server)
```

Minimum compilable consumer:

```cpp
#include "dlms/cosem/logical_device_interface.hpp"
#include "dlms/server/server_service_interface.hpp"
#include "dlms/server/server_types.hpp"

int main() {
  const dlms::server::ServerAssociationContext ctx =
    dlms::server::EmptyServerAssociationContext();
  return !ctx.associated ? 0 : 1;
}
```

Full runnable example: [`examples/package-consumers/cosem_server/`](../examples/package-consumers/cosem_server/).

## `dlms::runtime` — Client and endpoint facades

Pulls in: `dlms_client`, `dlms_endpoint`.
Use when you want the high-level `DlmsClient` + `MakeClientEndpoint` /
`MakeServerEndpoint` / `MakeGatewayEndpoint` / `MakePushListenerEndpoint`
facades without dragging in `dlms_cosem` server objects.

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_dlms_runtime_consumer LANGUAGES CXX)

find_package(DLMSFramework REQUIRED CONFIG COMPONENTS runtime)

add_executable(my_dlms_runtime_consumer main.cpp)
target_compile_features(my_dlms_runtime_consumer PRIVATE cxx_std_11)
target_link_libraries(my_dlms_runtime_consumer PRIVATE dlms::runtime)
```

Minimum compilable consumer:

```cpp
#include "dlms/client/client_options.hpp"
#include "dlms/endpoint/endpoint_options.hpp"

int main() {
  const dlms::client::DlmsClientOptions client =
    dlms::client::DefaultDlmsClientOptions();
  const dlms::endpoint::ClientEndpointOptions endpoint =
    dlms::endpoint::DefaultClientEndpointOptions();
  return client.requestTimeoutMs > 0 && endpoint.transport.timeoutMs > 0
    ? 0 : 1;
}
```

Full runnable example: [`examples/package-consumers/runtime/`](../examples/package-consumers/runtime/).

## `dlms::framework` — Everything

Pulls in `dlms::codec` + `dlms::io` + `dlms::protocol` + `dlms::cosem_server`
+ `dlms::runtime`. Use when you build a full client+server application or
just don't want to think about which sub-aggregate you need.

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_dlms_framework_consumer LANGUAGES CXX)

find_package(DLMSFramework REQUIRED CONFIG COMPONENTS framework)

add_executable(my_dlms_framework_consumer main.cpp)
target_compile_features(my_dlms_framework_consumer PRIVATE cxx_std_11)
target_link_libraries(my_dlms_framework_consumer PRIVATE dlms::framework)
```

Minimum compilable consumer:

```cpp
#include "dlms/client/client_options.hpp"
#include "dlms/endpoint/endpoint_options.hpp"

int main() {
  const dlms::client::DlmsClientOptions client =
    dlms::client::DefaultDlmsClientOptions();
  const dlms::endpoint::ClientEndpointOptions endpoint =
    dlms::endpoint::DefaultClientEndpointOptions();
  return client.requestTimeoutMs > 0 && endpoint.transport.timeoutMs > 0
    ? 0 : 1;
}
```

Full runnable example: [`examples/package-consumers/framework/`](../examples/package-consumers/framework/).

## Verification

The CMake snippets and the minimum `#include` sets above are kept in sync
with two automated checks:

- [`cmake/PackageInstallSmoke.cmake`](../cmake/PackageInstallSmoke.cmake)
  installs the package, then configures and builds three smoke consumers
  (full `framework`, `codec`-only, `io`-only) and the six per-aggregate
  examples under `examples/package-consumers/<aggregate>/`.
- [`dlms_package_install_smoke`](../cmake/PackageInstallSmoke.cmake) also
  asserts that the exported `DLMSFrameworkTargets.cmake` carries every
  documented `dlms::*` aggregate name and the documented
  `INTERFACE_LINK_LIBRARIES` for each aggregate, so the dependency graph
  shown at the top of this document is enforced by the build, not just by
  documentation.

If you add a new aggregate or change the per-aggregate component set, update:

1. `cmake/DlmsPackages.cmake` (the source of truth for aggregates).
2. `cmake/PackageInstallSmoke.cmake` (the `INTERFACE_LINK_LIBRARIES`
   assertions and any new consumer or example).
3. This document.
4. `examples/package-consumers/<aggregate>/` (CMakeLists + main.cpp).
