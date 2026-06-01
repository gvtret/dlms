# dlms-cosem

`dlms-cosem` owns the DLMS/COSEM object model used by the server side of the
framework. It provides logical devices, COSEM object identity, an object
registry, attribute and method access contracts, and association-view metadata.

The first implementation target is a minimal LN-referenced object model:

- model COSEM object identity by class id, version, and logical name;
- register objects inside one logical device;
- read object attributes through a stable interface;
- model attribute and method access rights;
- expose enough association-view data for a future `dlms-server` GET path.

The layer does not own xDLMS APDU parsing, association opening, transport,
ciphering, or public client/server facades.

## Documentation

- [Requirements](docs/00_cosem_requirements.md)
- [API](docs/01_cosem_api.md)
- [Architecture](docs/02_cosem_architecture.md)
- [Test Plan](docs/03_cosem_test_plan.md)
- [Implementation Plan](docs/04_cosem_implementation_plan.md)
