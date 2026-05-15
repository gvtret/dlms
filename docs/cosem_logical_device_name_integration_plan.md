# COSEM Logical Device Name Root Integration Plan

## 1. Scope

This phase connects the `dlms-cosem` Logical Device Name helper to the root
public client/server integration path.

The target path is:

```text
dlms-client -> dlms-xdlms -> dlms-server -> dlms-cosem::CosemDataObject
```

The object is the standard Logical Device Name Data object:

- class id: `1`
- logical name: `0.0.42.0.0.255`
- attribute: `2`

## 2. Requirements

1. Public-client GET integration shall read Logical Device Name attribute `2`
   from an object created by `MakeLogicalDeviceNameObject`.
2. The test shall register the object through the normal `LogicalDevice`
   registry and `dlms-server` path.
3. The root test shall compare encoded DLMS Data bytes only.
4. The phase shall not add a typed string decoder to root.

## 3. Architecture

```mermaid
sequenceDiagram
  participant Client as dlms-client
  participant XDlms as dlms-xdlms
  participant Server as dlms-server
  participant Device as LogicalDevice
  participant LDN as Logical Device Name Data object

  Client->>XDlms: GET class 1 / 0.0.42.0.0.255 / attr 2
  XDlms->>Server: decoded GET indication
  Server->>Device: resolve object
  Device->>LDN: ReadAttribute(2)
  LDN-->>Device: encoded Data octet-string
  Device-->>Server: encoded bytes
  Server-->>XDlms: GET response
  XDlms-->>Client: encoded bytes
```

```mermaid
classDiagram
  class DlmsClient
  class ClientServerApduChannel
  class XdlmsServerApduProcessor
  class DlmsServer
  class LogicalDevice
  class CosemDataObject
  class MakeLogicalDeviceNameObject

  DlmsClient --> ClientServerApduChannel
  ClientServerApduChannel --> XdlmsServerApduProcessor
  XdlmsServerApduProcessor --> DlmsServer
  DlmsServer --> LogicalDevice
  LogicalDevice --> CosemDataObject
  MakeLogicalDeviceNameObject --> CosemDataObject
```

## 4. Test Plan

- `ClientGetIntegration.PublicClientReadsLogicalDeviceName` shall:
  - create an LDN object with `MakeLogicalDeviceNameObject`;
  - register it on the logical device;
  - open the public client association;
  - GET class `1`, OBIS `0.0.42.0.0.255`, attribute `2`;
  - assert the returned bytes equal `ReadAttribute(2)` from the object.

## 5. Phase Exit Criteria

Documentation phase is complete when this plan is committed in root.

Implementation phase is complete when root MinGW build and root `ctest` pass.
