# dlms-endpoint

`dlms-endpoint` is the runtime composition layer for the DLMS/COSEM
framework.

It connects already implemented layers into executable client, server, push
listener, and gateway endpoints without moving their responsibilities:

- `dlms-transport` owns TCP, UDP, serial, timers, and tracing;
- `dlms-profile` owns Wrapper and HDLC/LLC APDU channels;
- `dlms-association` owns COSEM-OPEN, RELEASE, and association state;
- `dlms-xdlms` owns GET, SET, ACTION APDU service orchestration;
- `dlms-security` owns HLS and ciphered APDU processing;
- `dlms-client` and `dlms-server` own public facades and dispatch contracts;
- `dlms-cosem` owns the COSEM object model.

The endpoint layer owns lifecycle composition only: opening lower layers,
binding them to association and xDLMS processors, running one request cycle or
a bounded loop, and translating endpoint configuration into layer-specific
options.

Public runtime seams are abstract where applications commonly need custom
composition:

- `IApduChannelListener` for accepted server/push/gateway channels;
- `IPushIndicationHandler` for push delivery;
- `IGatewayPolicy` and `IGatewayUpstream` for gateway decisions and upstream
  forwarding;
- `dlms::server::IServerService` injection for `ServerEndpoint` and
  `ServerListenerRuntime`.

## Documentation

- [Requirements](docs/00_endpoint_requirements.md)
- [API](docs/01_endpoint_api.md)
- [Architecture](docs/02_endpoint_architecture.md)
- [Test plan](docs/03_endpoint_test_plan.md)
- [Implementation plan](docs/04_endpoint_implementation_plan.md)
