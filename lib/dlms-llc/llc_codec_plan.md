# План реализации LLC codec для DLMS/COSEM

## 1. Назначение документа

Документ описывает план реализации переносимой кроссплатформенной библиотеки C++11 для кодирования и декодирования **LLC sub-layer LPDU** в составе DLMS/COSEM HDLC-based communication profile.

Библиотека является тонким промежуточным слоем между APDU codec и HDLC/MAC layer:

```text
+-----------------------------+
| APDU codec                  |
+-----------------------------+
| LLC codec                   |
+-----------------------------+
| HDLC session                |
+-----------------------------+
| HDLC codec                  |
+-----------------------------+
| Transport: UART/TCP/etc.    |
+-----------------------------+
```

В первой версии реализуется только **LLC codec layer**, без APDU parsing, HDLC session state machine и transport logic.

---

## 2. Нормативная база и зафиксированные допущения

По IEC 62056-46 и Green Book Ed. 8.3:

```text
LLC sub-layer в connection-oriented HDLC profile основан на ISO/IEC 8802-2.
В DLMS/COSEM он используется как protocol selector.
LLC добавляет к LSDU три octets: DSAP, SSAP, LLC Quality/control.
LLC Quality/control currently shall be 0x00.
Destination LSAP 0xFF используется для broadcast receive; устройства не должны сами отправлять broadcast.
LLC fields отсутствуют в HDLC I-FRAGMENT и I-LAST-FRAGMENT, если LLC header уже был передан в I-FIRST-FRAGMENT.
```

Практические DLMS/COSEM HDLC vectors используют:

```text
client -> server: E6 E6 00 || xDLMS APDU
server -> client: E6 E7 00 || xDLMS APDU
```

Допущение v1: библиотека реализует HDLC-based LLC profile, а не IEC 61334-4-32 connectionless LLC profile для S-FSK PLC.

---

## 3. Зафиксированные проектные решения

| Вопрос | Решение |
|---|---|
| Ошибки | Только status-коды |
| Exceptions | Не используются в публичном API |
| Роль библиотеки | Универсальная: client + server |
| Основная функция | Add/remove 3-byte LLC header |
| APDU parsing | Не выполняется |
| HDLC frame parsing | Не выполняется |
| HDLC segmentation | Учитывается как boundary rule, не реализуется |
| Broadcast DSAP | Decode accept, encode forbidden by default |
| LLC Quality/control | v1 поддерживает только `0x00`, unknown возвращает unsupported/invalid |
| C ABI | Отдельный стабильный слой |
| CMake | Минимум 3.16 |
| Tests | GoogleTest |
| Buffers | `std::vector` convenience + caller-provided strict API |

---

## 4. Цель v1

Реализовать библиотеку, которая умеет:

```text
кодировать DLMS/COSEM LLC LPDU
декодировать DLMS/COSEM LLC LPDU
добавлять и удалять DSAP/SSAP/control header
различать client request и server response headers
принимать broadcast DSAP на decode path
запрещать broadcast DSAP на обычном encode path
проверять minimum LPDU size
сохранять APDU как opaque byte buffer
работать без exceptions
возвращать ошибки через status-коды
предоставлять C++11 API
предоставлять стабильный C ABI
собираться через CMake 3.16
иметь GoogleTest unit tests и root integration tests с HDLC
```

---

## 5. Границы v1

### 5.1. Входит в v1

```text
LLC header model
LSAP constants
LLC quality/control byte validation
LPDU encoder
LPDU decoder
client/server helper functions
strict no-allocation API
convenience vector API
C ABI wrapper
GoogleTest coverage
root integration tests with dlms-hdlc
documentation for segmentation boundary
```

### 5.2. Не входит в v1

```text
APDU codec
ACSE/xDLMS parsing
HDLC codec implementation
HDLC session state machine
HDLC segmentation/reassembly implementation
transport UART/TCP
WRAPPER codec
IEC 61334-4-32 connectionless LLC profile
DL-Reply / DL-Update-Reply services
security/ciphering
```

---

## 6. Рассмотренные подходы

### 6.1. Подход A - только fixed helper `prepend E6 E6 00`

Плюс: минимальный объем.
Минус: нет server response, нет validation, нет C ABI, легко размазать LLC knowledge по APDU/HDLC.

### 6.2. Подход B - самостоятельный LPDU codec

Плюс: маленький, тестируемый protocol selector с явным API и границами.
Минус: больше файлов, чем у one-off helper.

### 6.3. Подход C - LLC service state tables

Плюс: ближе к формальному описанию DL-CONNECT/DL-DATA.
Минус: быстро смешает LLC codec с HDLC session и MAC state machine.

### 6.4. Выбор

Выбирается **Подход B**:

```text
LPDU codec
+ role/direction helpers
+ validation
- LLC service state machine
- HDLC session logic
```

---

## 7. Структура проекта

```text
dlms-llc/
 ├── CMakeLists.txt
 ├── llc_codec_plan.md
 ├── include/
 │   └── dlms/
 │       └── llc/
 │           ├── llc_types.hpp
 │           ├── llc_error.hpp
 │           ├── llc_header.hpp
 │           ├── llc_codec.hpp
 │           └── llc_c_api.h
 ├── src/
 │   └── llc/
 │       ├── llc_header.cpp
 │       ├── llc_codec.cpp
 │       └── llc_c_api.cpp
 ├── test/
 │   ├── CMakeLists.txt
 │   └── llc/
 │       ├── test_llc_header.cpp
 │       ├── test_llc_codec.cpp
 │       ├── test_llc_c_api.cpp
 │       └── test_llc_vectors.cpp
 └── docs/
     ├── 00_llc_requirements.md
     ├── 01_llc_codec_api.md
     ├── 02_llc_c_api.md
     ├── 03_llc_segmentation_boundary.md
     └── 04_llc_test_plan.md
```

Корневой `E:/work/dlms` содержит интеграционные тесты:

```text
dlms/
 ├── CMakeLists.txt
 └── test/
     └── integration/
         ├── CMakeLists.txt
         └── test_llc_hdlc_integration.cpp
```

---

## 8. Status/error model

```cpp
enum class LlcStatus
{
  Ok,

  NeedMoreData,
  OutputBufferTooSmall,

  InvalidArgument,
  InvalidHeader,
  InvalidDsap,
  InvalidSsap,
  InvalidControl,
  InvalidLpduLength,

  LsduTooLarge,
  BroadcastEncodeForbidden,
  UnsupportedAddress,
  UnsupportedControl,
  UnsupportedFeature,

  InternalError
};
```

Правила:

```text
ни одна функция публичного API не бросает exceptions
strict API не аллоцирует память
vector API ловит allocation failures и возвращает InternalError
runtime path не вызывает abort/assert
```

---

## 9. Константы и модель LLC header

### 9.1. Constants

```cpp
static const std::uint8_t kLlcDsapDlms = 0xE6;
static const std::uint8_t kLlcSsapClient = 0xE6;
static const std::uint8_t kLlcSsapServer = 0xE7;
static const std::uint8_t kLlcDsapBroadcast = 0xFF;
static const std::uint8_t kLlcControlDefault = 0x00;
static const std::size_t kLlcHeaderSize = 3;
```

### 9.2. Header model

```cpp
enum class LlcEndpointRole
{
  Client,
  Server
};

enum class LlcDirection
{
  ClientToServer,
  ServerToClient
};

struct LlcHeader
{
  std::uint8_t dsap;
  std::uint8_t ssap;
  std::uint8_t control;
};
```

### 9.3. Направления

```text
ClientToServer:
  DSAP = 0xE6
  SSAP = 0xE6
  Control = 0x00

ServerToClient:
  DSAP = 0xE6
  SSAP = 0xE7
  Control = 0x00
```

Broadcast decode:

```text
DSAP = 0xFF may be accepted on decode path
SSAP still must identify a known peer SAP
```

---

## 10. LPDU model

### 10.1. Lightweight view

```cpp
struct LlcLpdu
{
  LlcHeader header;
  const std::uint8_t* lsduData;
  std::size_t lsduSize;
};
```

### 10.2. Owning container

```cpp
struct LlcLpduBuffer
{
  LlcHeader header;
  std::vector<std::uint8_t> lsdu;
};
```

Назначение:

```text
LlcLpdu       - view для encode/decode без владения памятью
LlcLpduBuffer - owning container для convenience API и тестов
```

---

## 11. C++ API

### 11.1. Header helpers

```cpp
LlcHeader MakeLlcHeader(LlcDirection direction);

bool IsKnownDlmsLlcHeader(const LlcHeader& header);

LlcStatus ValidateLlcHeader(
  const LlcHeader& header,
  bool allowBroadcastDestination);
```

### 11.2. Strict no-allocation API

```cpp
LlcStatus EncodeLpduToBuffer(
  const LlcHeader& header,
  const std::uint8_t* lsdu,
  std::size_t lsduSize,
  std::uint8_t* output,
  std::size_t outputSize,
  std::size_t& writtenSize);
```

```cpp
LlcStatus DecodeLpduFromBuffer(
  const std::uint8_t* input,
  std::size_t inputSize,
  bool allowBroadcastDestination,
  LlcLpdu& lpdu);
```

Decode view points into caller-owned `input`:

```text
lpdu.lsduData = input + 3
lpdu.lsduSize = inputSize - 3
```

### 11.3. Convenience API

```cpp
LlcStatus EncodeLpdu(
  const LlcHeader& header,
  const std::uint8_t* lsdu,
  std::size_t lsduSize,
  std::vector<std::uint8_t>& output);
```

```cpp
LlcStatus DecodeLpdu(
  const std::uint8_t* input,
  std::size_t inputSize,
  bool allowBroadcastDestination,
  LlcLpduBuffer& lpdu);
```

### 11.4. DLMS helpers

```cpp
LlcStatus EncodeDlmsRequest(
  const std::uint8_t* apdu,
  std::size_t apduSize,
  std::vector<std::uint8_t>& output);
```

```cpp
LlcStatus EncodeDlmsResponse(
  const std::uint8_t* apdu,
  std::size_t apduSize,
  std::vector<std::uint8_t>& output);
```

---

## 12. Segmentation boundary

LLC codec не режет и не собирает данные. Но он обязан зафиксировать boundary rules для HDLC integration:

```text
I-COMPLETE:
  information field contains full LLC LPDU

I-FIRST-FRAGMENT:
  information field starts with LLC header followed by first LSDU bytes

I-FRAGMENT:
  LLC header is absent, information contains continuation bytes

I-LAST-FRAGMENT:
  LLC header is absent, information contains final continuation bytes
```

Следствие:

```text
HDLC reassembler должен собрать полное information field до передачи в LLC decoder,
либо session layer должен передавать в LLC decoder только complete LPDU.
LLC decoder не должен угадывать missing header для continuation fragments.
```

---

## 13. C ABI

Файлы:

```text
include/dlms/llc/llc_c_api.h
src/llc/llc_c_api.cpp
```

Принципы:

```text
extern "C"
никаких C++ типов в ABI
никаких exceptions
только fixed-width integer types
caller-provided buffers
explicit status-name mapping, independent from C++ enum declaration order
```

Пример:

```c
typedef enum dlms_llc_status_t
{
  DLMS_LLC_STATUS_OK,
  DLMS_LLC_STATUS_NEED_MORE_DATA,
  DLMS_LLC_STATUS_OUTPUT_BUFFER_TOO_SMALL,
  DLMS_LLC_STATUS_INVALID_ARGUMENT,
  DLMS_LLC_STATUS_INVALID_HEADER,
  DLMS_LLC_STATUS_INVALID_DSAP,
  DLMS_LLC_STATUS_INVALID_SSAP,
  DLMS_LLC_STATUS_INVALID_CONTROL,
  DLMS_LLC_STATUS_BROADCAST_ENCODE_FORBIDDEN,
  DLMS_LLC_STATUS_UNSUPPORTED_FEATURE,
  DLMS_LLC_STATUS_INTERNAL_ERROR
} dlms_llc_status_t;
```

Тесты C API должны проверять возвращаемые именованные статусы по поведению, а
не порядок или числовые значения C++ `LlcStatus`.

```c
dlms_llc_status_t dlms_llc_encode_request(
  const uint8_t* apdu,
  size_t apdu_size,
  uint8_t* output,
  size_t output_size,
  size_t* written_size);
```

```c
dlms_llc_status_t dlms_llc_decode(
  const uint8_t* lpdu,
  size_t lpdu_size,
  uint8_t* dsap,
  uint8_t* ssap,
  uint8_t* control,
  const uint8_t** lsdu,
  size_t* lsdu_size);
```

---

## 14. CMake

```cmake
cmake_minimum_required(VERSION 3.16)

project(dlms_llc
  VERSION 0.1.0
  LANGUAGES C CXX)

option(DLMS_BUILD_TESTS "Build GoogleTest tests" ON)
option(DLMS_BUILD_C_API "Build stable C ABI wrapper" ON)
option(DLMS_USE_SYSTEM_GTEST "Use system-installed GoogleTest" OFF)

add_library(dlms_llc
  src/llc/llc_header.cpp
  src/llc/llc_codec.cpp)

target_compile_features(dlms_llc PUBLIC cxx_std_11)

target_include_directories(dlms_llc
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(DLMS_BUILD_C_API)
  target_sources(dlms_llc
    PRIVATE
      src/llc/llc_c_api.cpp)
endif()

if(DLMS_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
```

---

## 15. Тестовая стратегия

### 15.1. Header tests

```text
MakeHeader_clientToServer_returnsE6E600
MakeHeader_serverToClient_returnsE6E700
ValidateHeader_acceptsClientRequest
ValidateHeader_acceptsServerResponse
ValidateHeader_rejectsUnknownDsap
ValidateHeader_rejectsUnknownSsap
ValidateHeader_rejectsNonZeroControl
ValidateHeader_acceptsBroadcastDsapOnDecodeWhenAllowed
ValidateHeader_rejectsBroadcastDsapOnEncode
```

### 15.2. Codec tests

```text
EncodeRequest_prependsE6E600
EncodeResponse_prependsE6E700
Encode_emptyLsduAllowed
Encode_outputBufferTooSmall
Decode_requestReturnsLsduView
Decode_responseReturnsLsduView
Decode_zeroLengthNeedMoreData
Decode_oneOrTwoBytesNeedMoreData
Decode_invalidControlRejected
Decode_doesNotModifyApduBytes
Roundtrip_request
Roundtrip_response
```

### 15.3. C ABI tests

```text
CApi_encodeRequest
CApi_encodeResponse
CApi_decode
CApi_outputBufferTooSmall
CApi_noCrashOnNullArguments
CApi_broadcastDecodePolicy
```

### 15.4. Integration tests in root

```text
LlcHdlcIntegration_clientRequestLpduSurvivesHdlcRoundtrip
LlcHdlcIntegration_serverResponseLpduSurvivesHdlcRoundtrip
LlcHdlcIntegration_historicalDlmsVectorDecodesLlcHeader
LlcHdlcIntegration_rejectsHdlcInformationWithoutLlcHeader
LlcHdlcIntegration_rejectsUnsupportedLlcControlAfterHdlcDecode
LlcHdlcIntegration_payloadByte7eInsideApduSurvivesBothLayers
```

---

## 16. Реализационные фазы

### Фаза 0. Документы требований

Результат:

```text
llc_codec_plan.md
docs/00_llc_requirements.md
docs/01_llc_codec_api.md
docs/02_llc_c_api.md
docs/03_llc_segmentation_boundary.md
docs/04_llc_test_plan.md
```

Критерий готовности:

```text
LSAP values зафиксированы
LLC control policy зафиксирована
segmentation boundary явно описан
```

### Фаза 1. Каркас проекта

Результат:

```text
CMakeLists.txt
include/dlms/llc/*.hpp
src/llc/*.cpp
test/CMakeLists.txt
```

Критерий готовности:

```text
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

### Фаза 2. Status/error model

Результат:

```text
llc_error.hpp
```

Критерий готовности:

```text
единый LlcStatus
нет exceptions в публичном API
все invalid LPDU cases мапятся в status
```

### Фаза 3. Header constants and validation

Результат:

```text
llc_types.hpp
llc_header.hpp
llc_header.cpp
test_llc_header.cpp
```

Критерий готовности:

```text
E6/E6/00 и E6/E7/00 проходят
broadcast decode policy покрыта
invalid DSAP/SSAP/control покрыты
```

### Фаза 4. LPDU encoder

Результат:

```text
llc_codec.hpp
llc_codec.cpp
test_llc_codec.cpp
```

Критерий готовности:

```text
request/response LPDU кодируются
caller-provided buffers работают
output buffer too small обнаруживается
APDU bytes не меняются
```

### Фаза 5. LPDU decoder

Критерий готовности:

```text
valid LPDU декодируются
short input возвращает NeedMoreData
invalid header отклоняется
LSDU возвращается как view без копирования в strict API
```

### Фаза 6. C ABI

Критерий готовности:

```text
C API компилируется C-компилятором
не содержит C++ типов
покрыт тестами
использует caller-provided buffers
```

### Фаза 7. Root integration tests

Критерий готовности:

```text
LLC LPDU survives HDLC encode/decode roundtrip
реальный HDLC I-frame с LLC payload декодируется
invalid LLC payload after valid HDLC frame отклоняется LLC layer
payload byte 0x7E внутри APDU не ломает оба слоя
```

### Фаза 8. Doxygen public API documentation

Критерий готовности:

```text
каждая публичная функция документирована
каждое публичное поле структуры документировано
комментарии описывают ownership, buffer policy, LSAP constants и statuses
```

---

## 17. Основные риски

### 17.1. Смешивание LLC и HDLC session

Контроль:

```text
LLC не открывает/закрывает соединение
LLC не управляет sequence numbers
LLC не знает про retry/timeouts
LLC не выполняет HDLC segmentation
```

### 17.2. Неверная обработка HDLC fragments

Контроль:

```text
отдельный документ 03_llc_segmentation_boundary.md
root integration tests передают в LLC только complete LPDU
continuation fragments без header не декодируются как complete LPDU
```

### 17.3. Неверные LSAP constants

Контроль:

```text
constants в одном header
unit tests на E6 E6 00 и E6 E7 00
real HDLC vector с LLC payload
```

### 17.4. Broadcast misuse

Контроль:

```text
encode запрещает DSAP 0xFF по умолчанию
decode принимает 0xFF только при explicit allowBroadcastDestination
```

---

## 18. Итоговый milestone v1

```text
M1: Portable C++11 DLMS/COSEM LLC Codec
```

Состав:

```text
CMake 3.16
C++11
no exceptions
status-code API
client + server LPDU support
fixed DLMS/COSEM LLC header support
broadcast decode policy
strict no-allocation API
convenience vector API
stable C ABI
GoogleTest unit tests
root integration tests with dlms-hdlc
Doxygen-documented public API
```

Не входит:

```text
HDLC codec
HDLC session
transport
WRAPPER
APDU parser
IEC 61334-4-32 LLC
security/ciphering
```

---

## 19. Следующий практический шаг

Начинать реализацию следует с документов и минимального каркаса:

```text
1. docs/00_llc_requirements.md
2. docs/01_llc_codec_api.md
3. docs/02_llc_c_api.md
4. docs/03_llc_segmentation_boundary.md
5. docs/04_llc_test_plan.md
6. CMake project structure
7. empty library target
8. GoogleTest harness
9. LlcStatus
10. LlcHeader constants and validation
```

После этого переходить к LPDU encode/decode и C ABI.
