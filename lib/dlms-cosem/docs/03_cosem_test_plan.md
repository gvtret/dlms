# dlms-cosem Test Plan

## 1. Unit Tests

Phase 1 type and registry tests:

- default logical names are zeroed;
- object descriptors compare by class id and logical name;
- invalid descriptors are rejected;
- registry accepts unique object keys;
- registry rejects duplicate object keys;
- registry returns null or `ObjectNotFound` for missing objects;
- access-right helpers distinguish read, write, method, and authenticated
  modes.

Phase 2 object access tests:

- read attribute calls the object only when read access is granted;
- write attribute calls the object only when write access is granted;
- invoke method calls the object only when method access is granted;
- unsupported attributes return `AttributeNotFound`;
- unsupported methods return `MethodNotFound`;
- denied access returns `AccessDenied`;
- object callback errors return `ObjectError`;
- output buffers keep caller-owned data intact on failure.

Phase 3 logical-device tests:

- logical device stores SAP and registry;
- association view includes registered object descriptors and access rights;
- management device can expose SAP assignment metadata;
- association LN metadata can expose visible object list data.

Phase 5 simple interface object tests:

- Data object descriptor uses class id `1`, version `0`, and the configured
  logical name;
- Data attribute `1` returns encoded logical-name bytes;
- Data attribute `2` returns and writes the stored encoded value according to
  access rights;
- Register object descriptor uses class id `3`, version `0`, and the
  configured logical name;
- Register attribute `2` returns and writes the stored encoded value;
- Register attribute `3` returns the stored scaler-unit bytes;
- unsupported attributes return `AttributeNotFound`;
- methods return `MethodNotFound`;
- invalid logical names are rejected by existing registry descriptor validation.

Phase 7 association and SAP object tests:

- Association LN descriptor uses class id `15`, version `0`, and OBIS
  `0.0.40.0.0.255` for the helper default;
- Association LN attribute `1` returns encoded logical-name bytes;
- Association LN attribute `2` returns an encoded array of visible objects;
- object-list entries include class id, version, logical name, attribute access
  descriptors, and method access descriptors;
- SAP Assignment descriptor uses class id `17`, version `0`, and OBIS
  `0.0.41.0.0.255` for the helper default;
- SAP Assignment attribute `2` returns an encoded array of SAP/name pairs;
- logical-device name helper returns OBIS `0.0.42.0.0.255`;
- writes are denied by registry access checks;
- methods return `MethodNotFound`.

## 2. Integration Tests

Root integration is deferred until `dlms-server` exists. The first integration
test should verify a server-side normal GET path:

```text
dlms-server -> dlms-xdlms -> dlms-cosem -> ICosemObject
```

## 3. Verification Commands

Standalone:

```text
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Root:

```text
cmake -S . -B build-mingw64 -G "MinGW Makefiles"
cmake --build build-mingw64
ctest --test-dir build-mingw64 --output-on-failure
```
