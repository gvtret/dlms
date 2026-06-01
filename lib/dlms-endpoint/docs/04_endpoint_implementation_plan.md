# dlms-endpoint Implementation Plan

## Phase 0. Documentation

Deliverables:

- standalone `dlms-endpoint` repository;
- requirements;
- API contract;
- architecture diagrams;
- test plan;
- implementation plan;
- root architecture update documenting `dlms-endpoint` as the composition layer.

Commit message:

```text
docs(endpoint): define runtime composition layer
```

## Phase 1. Status And Options

Deliverables:

- `EndpointStatus`;
- transport/profile/security option structures;
- option defaults and validation helpers;
- focused unit tests.

Commit message:

```text
feat(endpoint): add status and option contracts
```

## Phase 2. Factories

Deliverables:

- transport factory over existing `dlms-transport` objects;
- profile factory over existing `dlms-profile` channels;
- security option adapter over existing `dlms-security` objects;
- fake-backed factory tests.

Commit message:

```text
feat(endpoint): compose transport profile and security factories
```

## Phase 3. Client Endpoint

Deliverables:

- `ClientEndpoint`;
- Wrapper/TCP no-security and password association path;
- GET, SET, ACTION forwarding to `dlms-client`;
- lifecycle tests with fake channels.

Commit message:

```text
feat(endpoint): add client endpoint facade
```

## Phase 4. Server Endpoint

Deliverables:

- `ServerEndpoint`;
- profile receive/send loop for one APDU;
- association metadata propagation into `ServerAssociationContext`;
- xDLMS APDU processing through `dlms-xdlms`;
- COSEM dispatch through `dlms-server`;
- fake-channel tests.

Commit message:

```text
feat(endpoint): add server endpoint run-once path
```

## Phase 5. Push Listener Endpoint

Deliverables:

- `IPushIndicationHandler`;
- `PushListenerEndpoint`;
- APDU receive and handler dispatch;
- error and lifecycle tests.

Commit message:

```text
feat(endpoint): add push listener endpoint
```

## Phase 6. Gateway Endpoint

Deliverables:

- `IGatewayPolicy`;
- `GatewayEndpoint`;
- downstream request to upstream client bridge;
- policy rejection tests;
- upstream error mapping tests.

Commit message:

```text
feat(endpoint): add gateway endpoint bridge
```

## Phase 7. Root Integration And Examples

Deliverables:

- root CMake integration;
- submodule pointer update;
- minimal SPODES client example;
- minimal SPODES server example;
- minimal push listener example;
- minimal gateway example;
- root deterministic integration tests.

Current root integration also executes the deterministic endpoint example
binaries through CTest so the examples are kept runnable, not only buildable.

Commit message:

```text
build: add endpoint layer and minimal examples
```
