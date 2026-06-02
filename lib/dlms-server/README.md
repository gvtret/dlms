# dlms-server

`dlms-server` is the server-side DLMS/COSEM orchestration layer.

The repository owns the boundary between decoded server-side xDLMS service
requests and the COSEM object model. It does not implement lower protocol
codecs, transport I/O, persistent object storage, or cryptographic primitives.

Implemented public contracts include:

- request and response models for GET, SET, and ACTION;
- `IServerService`, the abstract GET/SET/ACTION dispatch port;
- `DlmsServer`, the default dispatcher over `ServerContext` and
  `dlms-cosem::LogicalDevice`;
- `XdlmsServerAdapter`, which bridges `dlms-xdlms::IXdlmsServerHandler` to
  any `IServerService` implementation.

See `docs/` for requirements, API, architecture, test plan, and implementation
plan.

Minimal request-model usage:

```cpp
dlms::server::ServerContext context;
context.SetAssociated(true);
context.AttachLogicalDevice(&logicalDevice);

dlms::server::DlmsServer server(context);
dlms::server::ServerGetResponse response =
  server.HandleGet(getRequest);
```

Custom server backends can implement `dlms::server::IServerService` and pass
that implementation to `XdlmsServerAdapter` or higher endpoint composition
layers without using the default logical-device dispatcher. Such backends can
include `dlms/server/server_service_interface.hpp`; `dlms/server/dlms_server.hpp`
is needed only for the default `DlmsServer` facade.
