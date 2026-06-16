# Production-Ready Roadmap

## Цель

Довести DLMS/COSEM framework до состояния, где библиотека пригодна для
использования в продуктивных приложениях: публичные контракты стабильны,
ошибки предсказуемы, пакет устанавливается и потребляется извне, расширяемые
точки задокументированы, а критичные сценарии покрыты детерминированными
тестами.

## Принципы

- Сначала код, потом тесты, затем документация и версия.
- Каждый инкремент должен быть узким и проверяемым.
- Публичные C++ API, C ABI, CMake targets и documented behavior считаются
  совместимостью.
- Любой edge contract должен быть одинаковым внутри одного семейства API.
- Live meter проверки остаются opt-in; production-readiness не должна зависеть
  от внешнего оборудования.

## P0. Публичные контракты и edge behavior

1. Унифицировать `OutputBufferTooSmall` по codec strict API и C API.
   - `writtenSize`/`written_size`/`data_size` должны явно сообщать требуемый
     размер там, где API может его вычислить.
   - Малый буфер не должен приводить к частично записанному результату.
   - Уже закрыто для LLC encode, APDU raw xDLMS encode, Wrapper encode и
     HDLC encode.
   - HDLC C stream decoder и reassembler сообщают требуемый Information
     размер на small-buffer failures начиная с `0.3.48`.
   - Profile mutable receive для Wrapper TCP, Wrapper UDP и HDLC сообщает
     требуемый APDU размер на small-buffer failures начиная с `0.4.1`.
2. Завершить аудит очистки выходных структур C API.
   - Все выходные указатели очищаются до валидации.
   - Ошибки не оставляют старые значения в result/view/output структурах.
   - Profile receive boundary validation закрыта в `0.3.19`.
   - Profile callback adapters не передают ненулевой размер чтения после
     failed callback начиная с `0.3.36`.
   - Profile C API datagram callback receive coverage для failed callback с
     ненулевым callback byte count добавлен в `0.4.9`.
   - Transport C write/send entry points отклоняют null input с ненулевым
     размером начиная с `0.3.37`.
   - HDLC C decode допускает null information buffer только при нулевом
     размере начиная с `0.3.40`.
   - Security Setup method invocation очищает COSEM output на invalid
     activation и unsupported method failures начиная с `0.3.42`.
   - Built-in simple COSEM objects очищают output на missing attribute и
     missing method failures начиная с `0.3.43`.
   - COSEM object registry очищает output на missing object, access denied и
     object-level read/invoke failures начиная с `0.3.44`.
   - xDLMS server adapter очищает get/set/action result objects на
     status-level failures начиная с `0.3.45`.
   - xDLMS server dispatcher очищает get/set/action result objects на
     validation и handler failures начиная с `0.3.46`.
   - Endpoint security bundle factory очищает output bundle на validation
     failures начиная с `0.3.47`.
   - Association C API callback APDU adapter очищает receive output и
     отклоняет over-reported callback sizes начиная с `0.4.7`.
   - Transport C API read/receive entry points оставляют `bytes_read` равным
     нулю для non-OK backend statuses начиная с `0.4.8`.
3. Закрепить status mapping между слоями.
   - Не сводить полезные ошибки к `InternalError`, если есть более точный
     публичный статус. **Done** через серию `0.96.0`–`0.97.3`: facade
     mappers стали `switch`-exhaustive (`MapDataLinkDisconnectStatus`,
     `MapXdlmsStatus`, `MapAssociationStatus`, `MapClientStatus`,
     `MapProfileStatus`, `MapCosemStatus`); добавлены `ClientStatus`
     значения `BlockTransferRequired`, `InvokeIdMismatch`, `CodecFailed`.
   - Документировать намеренные потери детализации на facade уровнях.
     **Done в `0.97.7`**: новый раздел §1.1 “Facade Status Mapping Policy”
     в `lib/dlms-client/docs/01_client_api.md` фиксирует, что facade
     сохраняет (per-direction send/receive split, xDLMS block/invoke-id/
     codec failures как first-class), что осознанно сворачивает (per-layer
     transport detail, service-rejected reasons, security sub-classifi-
     cation, COSEM access-result vs. transport failure) и где живут
     mapper'ы.
4. Проверить все C headers smoke tests.
   - Каждый C ABI header должен компилироваться как C.
   - Stable enum numeric checks должны быть только там, где ABI это требует.
   - Wrapper C API stream decoder получил публичный push/drain entry point
     начиная с `0.4.0`.
   - **Закрыто в `0.98.2`**: каждый из 7 public `*_c_api.h` (apdu,
     association, hdlc, llc, profile, transport, wrapper) теперь
     имеет отдельный C-only smoke executable
     (`dlms_<mod>_c_header_smoke`) с собственным `main()`, который
     реально вызывает соответствующую функцию из чистого-C TU и
     зарегистрирован в ctest как `<Module>CApi.CHeaderCompilesAsC`.
     До этого 6 из 7 `test_*_c_header.c` файлов были compile-only
     canary внутри C++ gtest binary, без link-from-C-TU верификации.
5. Проверить install-tree export hygiene.
   - Concrete CMake targets экспортируют package include directory без
     дублирования начиная с `0.4.2`.
   - `find_package(DLMSFramework COMPONENTS codec)` больше не требует OpenSSL
     начиная с `0.4.3`; install smoke проверяет codec-only consumer with
     OpenSSL disabled.
   - `find_package(DLMSFramework COMPONENTS io)` больше не требует OpenSSL
     начиная с `0.4.4`; install smoke проверяет io-only consumer with
     OpenSSL disabled.

## P0. Endpoint, client и server lifecycle

1. Проверить idempotency `Open()`, `Close()`, `RunOnce()` и listener runtime.
   - Статус: listener runtime `Open()`/`Close()` coverage уже есть;
     server, push listener и gateway endpoint repeated `Open()` coverage
     добавлен в `0.3.25`.
2. Проверить ошибки при частично открытых transport/profile/association цепочках.
   - Статус: server и push listener endpoint close-failure coverage добавлен
     в `0.3.26`; gateway downstream close-failure coverage добавлен в
     `0.3.27`; server и push listener malformed negotiated open retry
     coverage добавлен в `0.3.28`; high-password HLS retry after invalid
     reply coverage добавлен в `0.3.29`; HLS GMAC retry after invalid reply
     coverage добавлен в `0.3.30`; remaining cleanup coverage теперь
     отслеживается в listener runtime и Security Setup задачах.
3. Добавить regression tests для cleanup при неуспешном open/association.
   - **Закрыто** в `0.98.0`–`0.98.1`:
     - `0.98.0`: 4 теста в `test_client_endpoint.cpp`
       (`OpenAfterFailedOpenIsIdempotentAndRetries`,
       `CloseAfterFailedOpenLeavesNoStateBehind`,
       `OpenIsIdempotentAfterValidationFailure`,
       `DestructorClosesAfterFailedOpenWithoutLeak`) + real-bug fix
       в `ClientEndpoint::Close()` (при non-Ok `client->Close()` инстанс
       оставался привязанным, что приводило к leak'у через
       move-assignment при следующем `Open()`; теперь всегда
       сбрасывается, статус сохраняется и возвращается каллеру).
     - `0.98.1`: 3 теста для серверных endpoint'ов
       (`ServerEndpoint`, `PushListenerEndpoint`, `GatewayEndpoint`):
       failed channel-open возвращает статус и оставляет endpoint
       закрытым; следующий `Open()` реально перезапрашивает канал,
       не short-circuit'ит на stale state; в gateway-варианте
       дополнительно верифицируется, что upstream не открывается
       при downstream-failure. Попутно исправлены три test-only
       `FakeApduChannel::Open()` fixture'а (ставили `open=true` даже
       при ошибке).
4. Проверить, что bounded loops не скрывают `Timeout`, `Closed`, `InvalidState`.
   - Аудит пройден в `0.97.3`: проверены все `for(;;)` и
     `while(...)` циклы в `lib/dlms-*` источниках. Каждый цикл
     либо проводит receive-статус наверх как есть (через
     `if (status != Ok) return status;`), либо не связан с
     транспортом вовсе (`EncodeData` grow loop в client,
     ber/axdr decode loops в apdu, segmentation loops в hdlc).
     `for(;;)` в xDLMS клиенте на блочной передаче (Get/Set/
     Action) пробрасывают любой не-`Ok` от `SendAndReceive` /
     `ReceiveGetResponse` / `ReceiveActionResponse` без потерь.

## P0. Security и секреты

По базе знаний СПОДЭС/СПОДУС и DLMS/COSEM security не сводится к HLS GMAC:
нужны Security Setup IC `64`, suites `0/1/2`, управление ключами,
сертификатами, dedicated/general ciphering и строгая политика invocation
counter. Текущий код покрывает только часть Suite 0: low password,
HLS password/GMAC, AES-GCM ciphered APDU и локальные key/counter stores.
Текущий статус зафиксирован в `docs/security_support_matrix.md`.

1. Зафиксировать supported/unsupported security matrix.
   - Suite 0: `None`, Low, High password, High GMAC, global ciphering.
   - Suite 0 gaps: `change_HLS_secret`, dedicated key и dedicated ciphering
     APDU; Security Setup IC `64`, `security_activate` и
     `global_key_transfer` реализованы частично, но certificate/key agreement
     semantics остаются unsupported.
   - Статус: Suite 0 AES key wrap/unwrap primitive добавлен в `0.3.31`;
     mutable key sink добавлен в `0.3.32`; Security Setup method 2 Suite 0
     key-transfer parsing добавлен в `0.3.33`; invocation-counter reset
     policy hook добавлен в `0.3.34`; malformed key-transfer parser
     coverage для unsupported key id и trailing bytes добавлен в `0.3.35`;
     unsupported suite key-transfer coverage добавлен в `0.3.38`.
   - Suite 1/2 gaps: ECDSA HLS, ECDH key agreement, certificates, key
     agreement methods, AES-GCM-256 and SHA-384.
2. Реализовать Security Setup IC `64` как COSEM extension point.
   - Attributes: security policy, security suite, client/server system title,
     certificates where applicable.
   - Methods: `security_activate`, key transfer, key agreement and certificate
     operations with unsupported statuses until implemented.
   - Policy must be monotonic: activation may strengthen but must not weaken
     the configured security policy.
   - Статус: read-only attributes для policy, suite и system titles добавлены
     в `CosemSecuritySetupObject`; `security_activate` выполняет monotonic
     policy strengthening; Suite 0 AES key wrap/unwrap primitive добавлен в
     `0.3.31`; mutable key sink добавлен в `0.3.32`; method 2 Suite 0 key
     transfer wiring добавлен в `0.3.33`; invocation-counter reset policy
     hook добавлен в `0.3.34`; malformed key-transfer parser coverage
     добавлен в `0.3.35`; unsupported suite key-transfer coverage добавлен
     в `0.3.38`; unsupported Security Setup method range coverage добавлен
     в `0.3.39`; reset policy gate перед установкой ключей добавлен в
     `0.3.41`; output cleanup для unsupported method failures добавлен в
     `0.3.42`; certificate и key agreement semantics остаются следующими
     задачами.
3. Invocation counter production contract.
   - IV is `system_title[8] || invocation_counter[4]`.
   - Counter must be monotonic for each key/system-title context.
   - Reject received protected APDU when counter is not greater than the last
     accepted value for the sender.
   - Refuse encryption when counter reaches `2^32 - 1`; require key rotation.
   - Expose/read the public invocation counter object
     `0.0.43.1.0.255`, class id `1`, where profile policy requires it.
   - Статус: local counter exhaustion отказ покрыт для protected APDU и
     HLS GMAC response paths в `0.3.22`; public invocation counter Data
     object helper добавлен в `0.3.23`; in-memory per-sender replay state
     добавлен в `0.3.24`. Durable persistence и key rotation behavior
     остаются задачами.
4. Ciphering completeness.
   - General global ciphering and service-specific global ciphering.
   - Dedicated ciphering and dedicated key lifetime scoped to association.
   - `glo_*`, `ded_*`, and general-ciphering APDU coverage in APDU/xDLMS.
5. HLS completeness.
   - Keep HLS GMAC vectors for Suite 0.
   - Add HLS SHA-256 mechanism where required by supported deployments.
   - Add HLS ECDSA only after Suite 1/2 certificate/key infrastructure exists.
6. Secret handling.
   - Trace/log paths must not output keys, passwords, challenges, GMAC input,
     wrapped keys or plaintext protected APDU unless explicitly requested for a
     test-only diagnostic build. **Done in `0.97.4`** for the live smoke tool:
     wire-byte hex dumps in `tools/live_meter_smoke` are now gated behind
     `DLMS_LIVE_TRACE_WIRE_BYTES=1`. `AssociationTraceEvent` was reviewed and
     already publishes only the calling-authentication-value size, never the
     bytes themselves. `lib/` trace sink interfaces intentionally hand the
     application raw bytes via hooks; redaction is the consumer’s job.
   - Add redaction tests for endpoint/client live-smoke diagnostics.
     **Done in `0.97.5`**: extracted the wire-byte policy to
     `tools/live_meter_smoke_byte_emit.hpp` and added
     `dlms_live_meter_smoke_redaction_tests` (10 cases) under the existing
     `DLMS_BUILD_LIVE_TESTS` guard. The test pins default-off,
     opt-in-only, non-wire-events-silent, and empty-byte-span-silent.
   - Document storage, ownership and lifetime for key stores and invocation
     counter stores. **Done in `0.97.6`**: see new section §5.1
     “Storage, Ownership and Lifetime” in
     `lib/dlms-security/docs/01_security_api.md`.

## P0. COSEM IC и СПОДЭС/СПОДУС model coverage

База знаний указывает, что для СПОДЭС/СПОДУС одних protocol services
недостаточно. Нужен прикладной объектный слой с обязательными IC, OBIS
каталогами, access rights, Profile Generic структурами и наборами параметров.
Текущий `dlms-cosem` реализует минимальный generic object model и часть
Data/Register/Association LN/SAP Assignment behavior, но не является полной
СПОДЭС/СПОДУС моделью.
Текущий статус зафиксирован в `docs/ic_support_matrix.md`.
Сверка `0.4.11` по базе знаний подтвердила, что built-in IC coverage был
ограничен Data `1`, Register `3`, Association LN `15`, SAP Assignment `17` и
частичным Security Setup `64`; `0.5.0` добавляет partial Profile Generic `7`,
а `0.13.0` добавляет partial Clock `8`.
Остальные IC из ГОСТ Р 58940-2020 table 7.1 и DLMS UA Blue Book должны
оставаться `Planned` или `Application-provided` до явной реализации.

1. Ввести explicit IC support matrix.
   - Для каждого IC: class id, supported versions, attributes, methods,
     access modes, encode/decode status.
   - Первые группы: Data `1`, Register `3`, Extended Register `4`,
     Demand Register `5`, Profile Generic `7`, Clock `8`, Script Table `9`,
     Schedule `10`, Special Days Table `11`, Association LN `15`,
     SAP Assignment `17`, Image Transfer `18`, IEC HDLC Setup `23`,
     Push Setup `40`, TCP-UDP Setup `41`, Security Setup `64`,
     Disconnect Control `70`, Limiter `71`.
2. Реализовать IC registry/factory.
   - Пользователь должен иметь возможность зарегистрировать собственную IC
     реализацию или заменить встроенную.
   - Unknown class id должен возвращать предсказуемый `object-unavailable` или
     `type-unmatched`, а не падать в generic error.
3. Profile Generic.
   - Capture objects, buffer, selective access, scaler/unit normalization,
     timestamp handling.
   - СПОДЭС/СПОДУС journals: discovered meters, exchange statuses,
     object-correction log, meter parameter journal, event aggregation profiles.
   - Статус: partial built-in `CosemProfileGenericObject` с read-only
     attributes, encoded buffer rows и capture objects добавлен в `0.5.0`;
     `0.9.0` добавляет class-level encode/decode helpers для составных
     атрибутов `buffer`, `capture_objects` и `sort_object`; `0.10.0`
     добавляет range и entry selective access descriptor helpers. Capture
     execution и fixed journal schemas остаются задачами.
   - Для каждого составного атрибута IC (`array`, `structure`) встроенная IC
     реализация должна публиковать class-level encode/decode helpers рядом с
     объектом класса, чтобы прикладной код не разбирал A-XDR вручную.
4. Clock.
   - Clock `8` нужен GUI-клиенту для чтения и настройки времени, часового
     пояса, статуса, daylight-saving параметров и clock base.
   - Статус: `0.13.0` добавляет partial built-in `CosemClockObject` с
     read/write поддержкой документированных атрибутов `1`-`9`. Атрибуты
     `time`, `daylight_savings_begin` и `daylight_savings_end` валидируются как
     DLMS Data `octet-string` с 12 байтами date-time. Методы `1`-`6`
     возвращают `UnsupportedFeature`, пока не зафиксирована политика
     корректировки времени.
5. СПОДЭС/СПОДУС catalogs.
   - OBIS catalogs and parameter lists for meter categories A/B/C/D and ИВКЭ.
   - Event code table and status word formats.
   - Access policy profiles for public reader/configurator modes.
6. Association LN object list and access rights.
   - Object list must reflect visible COSEM objects and access rights for the
     current association/security context.
   - Статус: `0.11.0` добавляет class-level encode/decode helpers для
     `object_list` и `access_rights` structures; `0.14.0` добавляет
     `association_status`, optional `security_setup_reference` и явные
     unsupported statuses для методов `reply_to_HLS_authentication`,
     `change_HLS_secret`, `add_object`, `remove_object`; `0.15.0` переводит
     Association LN на caller-selected version-gated модель до версии `3` и
     добавляет user-list/current-user surface для версии `2+`; `0.17.0`
     добавляет caller-selected descriptor version constructors и
     `MaxSupportedVersion` для Data, Register, Clock, Profile Generic,
     SAP Assignment и Security Setup, а Security Setup по умолчанию публикует
     версию `1`; Profile Generic v0 методы `3`/`4` и Security Setup v0/v1
     методы gated по версии класса; `0.18.0` добавляет pluggable
     `ICosemCertificateStore` backend для Security Setup v1 attribute `6`
     `certificates` (DLMS Data array of `certificate_info` structures) и
     реализует методы `6` (`import_certificate`), `7` (`export_certificate`)
     и `8` (`remove_certificate`) с парсингом Blue Book `by_entity`/
     `by_serial` selector структур и in-memory reference backend.
   - Остаются `associated_partners_id`, application context name,
     xDLMS context info, authentication mechanism name, secret handling и
     выполнение HLS/object add/remove methods; X.509 парсинг для
     автозаполнения subject/issuer/serial при `import_certificate` остаётся
     TODO.
7. Push setup and initiative messages.
   - Push Setup IC `40` version handling.
   - Notification payload structures required by СПОДЭС/СПОДУС.
   - Rule: transmit only actual data not yet sent/confirmed where this is part
     of ИВКЭ behavior.
8. Image Transfer and control classes.
   - Image Transfer IC `18` block flow.
   - Disconnect Control `70`, Limiter `71`, schedules and script execution
     needed by meter operations.

## P0. Протокольная полнота MVP

1. Association lifecycle: open, release, abort, low password, HLS GMAC.
2. xDLMS GET/SET/ACTION: normal и block transfer в обе стороны.
3. Server path: COSEM object lookup, access rights, association view, logical
   device name, SAP assignment.
4. Profile path: Wrapper/TCP, Wrapper/UDP, HDLC over byte stream, HDLC session.
5. Client facade и endpoint facade: сценарии client, server, push listener,
   gateway.
6. GUI client replacement surface: stable `DlmsClient` calls by class id,
   OBIS logical name and attribute/method id, with detailed access/action
   result reporting. Статус: `ReadAttribute`, `WriteAttribute` и `CallMethod`
   added in `0.6.0`; common typed DLMS Data encode/decode helpers added in
   `0.7.0`; generic xDLMS/client GET selective access added in `0.8.0`;
   Profile Generic selector-specific helpers added in `0.10.0`; DLMS
   `date-time`, `date`, and `time` typed helpers added in `0.12.0`;
   СПОДЭС OBIS GUI read example added in `0.12.1`; partial Clock `8`
   attribute object added in `0.13.0`.

## P0. СПОДЭС/СПОДУС completeness gate

Production-ready status must be split into two claims:

1. DLMS/COSEM transport/protocol framework readiness.
2. СПОДЭС/СПОДУС application model readiness.

The second claim is blocked until the IC support matrix, security matrix and
СПОДЭС/СПОДУС catalogs have automated conformance checks. Until then the
library may be described as an extensible DLMS/COSEM framework with partial
СПОДЭС/СПОДУС support, not as a complete СПОДЭС/СПОДУС implementation.

## P1. Transport и runtime

1. Решить статус TLS.
   - Либо реализовать `TlsStreamTransport`, либо документировать как
     unsupported и не позиционировать как production surface.
2. Проверить non-blocking interfaces и event loop на реальные сценарии.
3. Добавить tests для serial edge cases и IEC 62056-21 Mode E.
4. Оптимизировать buffer reuse в profile/endpoint paths, где сейчас есть
   повторные временные vectors.

## P1. Package и consumer experience

1. Расширить install-tree examples.
   - Уже есть `codec`, `io`, `protocol`, `cosem_server`, `runtime`,
     `framework` начиная с `0.4.5`.
   - README snippets для всех component/target пар добавлены в `0.4.6`.
2. Продолжать exported target audit.
   - Include dirs, transitive deps, OpenSSL dependency, отсутствие test deps.
   - Aggregate target `INTERFACE_LINK_LIBRARIES` проверяются install smoke
     начиная с `0.4.10`.
3. ✅ DONE (v0.105.2): минимальные include примеры для каждого aggregate
   target сведены в `docs/package_consumer_minimum.md`. Для всех 6 публичных
   `dlms::*` aggregates (`codec`, `io`, `protocol`, `cosem_server`,
   `runtime`, `framework`) задокументированы: CMake-snippet с
   `find_package` + `target_link_libraries`, минимальный compilable
   `main.cpp`, что несёт каждый aggregate (transitive components),
   OpenSSL-зависимость, ссылки на runnable `examples/package-consumers/`.
   Привязка к verification: `cmake/PackageInstallSmoke.cmake` уже строит
   все 6 примеров, поэтому документ автоматически синхронизирован с
   рабочими сборками.
4. Проверить package artifact на Windows/MSYS2 и Linux CI, если Linux runner
   доступен.

## P1. Диагностика

1. ✅ DONE (v0.99.1): trace contracts упорядочены. Карта 4 публичных sink-ов
   (`ITransportTraceSink`, `IWrapperTcpTraceSink`, `IHdlcProfileTraceSink`,
   `IAssociationTraceSink`) с per-sink контрактом, security-семантикой
   (что несёт raw bytes, что — только размеры), framework guarantees
   (lifecycle, re-entrancy, span-validity, no-exception-escape) и
   consumer responsibilities (redaction, thread safety, backpressure,
   filtering) сведена в `docs/trace_contracts.md`. Endpoint reuse паттерн
   задокументирован.
2. ✅ DONE (v0.99.3 design → v0.99.4 / v0.99.5 / v0.99.6): correlation
   metadata для multi-layer traces без раскрытия секретов.
   - `v0.99.3`: дизайн в `docs/trace_correlation_design.md`.
   - `v0.99.4` (commit 1/3): `MakeConversationId(seed, invokeId)`
     primitive в `dlms-xdlms` + 6 unit-тестов
     (`test_xdlms_correlation.cpp`) фиксируют детерминированность,
     устойчивость к коллизиям и опакность.
   - `v0.99.5` (commit 2/3): `conversationId` (uint64) добавлен
     последним полем во все 4 trace event structs (`TransportTraceEvent`,
     `AssociationTraceEvent`, `WrapperTcpTraceEvent`,
     `HdlcProfileTraceEvent`) с default-init `= 0` (kNoConversationId),
     plus `IApduChannel::SetCorrelation(uint64_t) noexcept` default
     no-op. POD append — не ломает ABI в 0.x. 7 регрессионных тестов
     в новом `test_trace_correlation.cpp` пинят contract.
   - `v0.99.6` (commit 3/3): `WrapperTcpProfileChannel` и
     `HdlcProfileChannel` override-ят `SetCorrelation()` и стампят
     `conversationId` на каждое emit'нутое событие (4 sites в каждом
     cpp). Integration-тесты `WrapperTcpChannelStampsSendAndReceiveEvents`
     и `HdlcChannelStampsWireWriteEvents` гоняют каналы через
     `FakeByteStream`, флипают `SetCorrelation` on/off и пинят stamping
     по всем событиям в окне. Test fixture total: 9 cases. Full ctest:
     967/967.
   - `v0.100.0` (wiring closure): `XdlmsClient::Get/Set/Action` теперь
     сами вызывают `channel_.SetCorrelation(MakeConversationId(seed,
     invokeId))` сразу после выделения invoke id и до первого
     `SendApdu`, используя seed из нового virtual
     `IXdlmsAssociationState::ConversationSeed() const noexcept`
     (default = 0, ABI-safe). Канальные sink'и (см. v0.99.6) видят
     тот же `conversationId`, что и xDLMS sink (`IXdlmsTraceSink`,
     §7 commit 1/3 в той же версии), для одного service call —
     без ручного prop'а в потребительском коде. 3 новых
     integration-теста в `test/integration/test_cross_layer_correlation.cpp`
     (`GetEmitsMatchingConversationIdOnBothLayers`,
     `ZeroSeedYieldsZeroConversationId`,
     `NoTraceSinkStillCallsSetCorrelation`) пинят инвариант end-to-end
     через custom `IApduChannel`, который захватывает `SetCorrelation`
     и стампит `WrapperTcpTraceEvent` с этим id. Full ctest: 970/970.
   Секреты не используются: seed — opaque логгинг-соль, не system-title
   и не HLS challenge.
   Follow-up (отдельная задача, не блокер §2): production-ассоциации
   (`AssociationClient`) пока возвращают `ConversationSeed() == 0`, то
   есть `conversationId == kNoConversationId` для всех реальных
   деплоев — поведение sink'ов не меняется. Когда `AssociationClient`
   начнёт публиковать ненулевой seed (через `AssociationOptions` или
   на основе context info), вся propagation-цепочка от xDLMS до
   каналов уже работает и подхватит id без изменений в самих каналах.
3. ✅ DONE (v0.98.3 + v0.99.0): status-to-string полнота во всех публичных
   status enum. Все 11 публичных status enum (`ApduStatus`,
   `AssociationStatus`, `ClientStatus`, `CosemStatus`, `EndpointStatus`,
   `HdlcStatus`, `LlcStatus`, `ProfileStatus`, `SecurityStatus`,
   `ServerStatus`, `TransportStatus`, `WrapperStatus`, `XdlmsStatus`) теперь
   имеют публичный `*StatusName()` (или `ToString()`) helper с exhaustive
   switch без `default:` arm (`-Wswitch` гарантирует, что новые enum value
   не забудут строку) и gtest-покрытием каждого enum value + `Unknown`
   fallback. v0.98.3 — exhaustive coverage tests для 6 уже существующих
   helper-ов (ApduStatus/AssociationStatus/CosemStatus/ServerStatus/
   XdlmsStatus/EndpointStatus, +10 тестов). v0.99.0 — добавлены 4 новых
   публичных helper-а для wire-layer enum (HdlcStatus/LlcStatus/
   WrapperStatus/ProfileStatus, +8 тестов). Дубликат `ProfileStatusName` из
   `tools/live_meter_smoke.cpp` удалён.
4. ✅ DONE (v0.97.4 + v0.97.5): live smoke output safe by default. Wire-byte
   hex dump в `tools/live_meter_smoke.cpp` теперь gated env-флагом
   `DLMS_LIVE_TRACE_WIRE_BYTES` (off by default); 10 regression-тестов
   зафиксировали политику redaction.
5. Привести `*StatusName` к единообразному именованию (сейчас
   `EndpointStatus` использует `ToString()` вместо `EndpointStatusName()` —
   это inconsistency, но переименование = BREAKING; запланировать на
   следующий major bump или ввести alias).
6. ✅ DONE (v0.99.2): API docs of every layer reference `*StatusName()`
   as a stable diagnostic contract (not for parsing). Per-module
   `01_*_api.md` now ends with a "Diagnostic helpers" section that names
   the helper and links to `docs/status_to_string_contract.md`, the
   single canonical contract document covering totality, `"Unknown"`
   fallback, lifetime, thread-safety, no-allocation, ABI stability, and
   the catalogue of 13 helpers + matching coverage test files.
7. ✅ DONE (v0.99.7 design → v0.100.0 commit 1/3 → v0.101.0 commit 2/3 → v0.102.0 commit 3a → v0.104.0 commit 3b → v0.105.0 commit 3c):
   публичный xDLMS-layer trace sink и server-side trace sink. v0.99.7
   зафиксировал дизайн в `docs/xdlms_server_trace_design.md` (два sink-а:
   `IXdlmsTraceSink` в `dlms-xdlms` для xDLMS слоя, client + server;
   `IServerDispatchTraceSink` в `dlms-server` для dispatcher-ровня;
   12 event kinds; sizes и identifiers только, never raw bytes;
   все events несут `conversationId`).
   v0.100.0 — commit 1/3 (client side): `IXdlmsTraceSink`,
   `XdlmsTraceEvent`, `XdlmsTraceKind` (12 kinds) опубликованы в
   `lib/dlms-xdlms/include/dlms/xdlms/xdlms_trace.hpp`. `XdlmsClient`
   получил opt-in `SetTraceSink`/`TraceSink` (без новых ctor
   overloads, ABI-safe) и эмитирует Request/Response/DecodeFailed/
   SecurityFailed из единого `SendAndReceive` сайта, плюс
   BlockTransferStep в блочных циклах Get/Set/Action и
   InvokeIdRejected при invoke-id mismatch.
   v0.101.0 — commit 2/3 (server side): `XdlmsServerApduProcessor`
   получил тот же opt-in `SetTraceSink`/`TraceSink` (ABI-safe:
   `traceSink_` поле добавлено в конец, инициализируется в `0` во
   всех 6 конструкторах). Эмитирует с `XdlmsTraceDirection::Inbound`:
   `SecurityFailed` (Unprotect/Protect fail), `DecodeFailed`
   (`DecodeXdlmsApdu` fail), `RequestReceived` (после успешного
   decode, с invokeId/options/classId/attribute or method id/OBIS LN
   для Get/Set/Action), `ResponseSent` (после успешного encode и
   возможного `Protect`), `InvokeIdRejected` и `BlockTransferStep` —
   из per-service block-transfer helpers. Серверная сторона пока
   ставит `conversationId = kNoConversationId`; endpoint composition
   опубликует id в commit 3/3. Новый тест-файл
   `lib/dlms-xdlms/test/xdlms/test_xdlms_server_trace.cpp` (3 cases).
   v0.102.0 — commit 3a (server-dispatch sink + endpoint wiring):
   `IServerDispatchTraceSink` и `ServerDispatchTraceEvent` опубликованы
   в `lib/dlms-server/include/dlms/server/server_dispatch_trace.hpp`
   (три kind-а: `GetDispatched`, `SetDispatched`, `ActionDispatched`).
   Pass-through декоратор `TracingXdlmsServerDispatcher` (в
   `dlms/server/tracing_xdlms_server_dispatcher.hpp`) оборачивает
   любой `IXdlmsServerDispatcher` и эмитирует ровно одно событие на
   каждый Dispatch*; при `sink == nullptr` zero-cost. Endpoint
   composition: `ServerEndpointOptions` (и `GatewayEndpointOptions::
   downstream` через него) получили два опциональных pointer-поля
   `xdlmsTraceSink` и `serverDispatchTraceSink`, которые
   пробрасываются в `ServerEndpoint::ConfigureXdlmsProcessor` и
   `GatewayEndpoint`. `PushListenerEndpointOptions` и
   `ClientEndpointOptions` намеренно НЕ расширены (push-listener
   не запускает диспетчер; client-side доступен через прямой
   `XdlmsClient::SetTraceSink` для embedded-кода — без реального
   потребителя добавлять speculative API нарушило бы AGENTS.md §2).
   v0.104.0 — commit 3b (end-to-end `conversationId` на серверной
   стороне xDLMS): `XdlmsServerApduProcessor` после `DecodeXdlmsApdu`
   считает `conversationId = MakeConversationId(seedSource_->
   ConversationSeed(), invokeId)` и стампит его на каждое emit-нутое
   `IXdlmsTraceSink` event-а для этого запроса (`RequestReceived`,
   `ResponseSent`, inner `InvokeIdRejected` / `BlockTransferStep` из
   `ProcessGetRequest`/`ProcessSetRequest`/`ProcessActionRequest`,
   post-decode `SecurityFailed` от Protect). События, эмитируемые
   до того, как invokeId известен (Unprotect-failure `SecurityFailed`,
   `DecodeFailed`), по-прежнему несут `kNoConversationId`. Если у
   processor-а установлен `IApduChannel*` через `SetApduChannel(...)`,
   тот же id одновременно ставится на канал через
   `channel_->SetCorrelation(conversationId)` — это даёт
   transport-layer trace sink-ам (`HdlcProfileChannel`,
   `WrapperTcpProfileChannel`) стабильный id для всех outbound
   response APDU в этом lifecycle. Helpers `EmitServerTrace` /
   `EmitServerSimpleTrace` и три `Process*Request` получили внутренний
   trailing `conversationId` параметр (private, без публичного API
   impact-а). Тесты: два новых case-а в
   `lib/dlms-xdlms/test/xdlms/test_xdlms_server_trace.cpp` —
   `ServerStampsConversationIdOnEventsAndChannel` (pin: и channel,
   и оба event-а получают `MakeConversationId(seed, invokeId)`) и
   `ServerZeroSeedYieldsInvokeIdAsConversationId` (pin документ-
   ированного свойства `MakeConversationId(0, invokeId) ==
   invokeId & 0x0F`, mirror клиентского
   `CrossLayerCorrelation.ZeroSeedYieldsZeroConversationId`). Полный
   ctest: 974/975 зелёные (pre-existing MinGW multi-config артефакт
   `dlms_package_install_smoke` остаётся единственным красным —
   независимо от §7).
   v0.105.0 — commit 3c (end-to-end `conversationId` на dispatch
   sink + integration test): `TracingXdlmsServerDispatcher` получил
   опциональный back-reference на `IApduChannel` через
   `SetCorrelationChannel(IApduChannel*)` / `CorrelationChannel()`.
   При установленном channel декоратор читает
   `channel->CurrentConversationId()` (опубликован в 0.103.0) и
   стампит его на каждое `ServerDispatchTraceEvent` (Get/Set/Action);
   при `channel == nullptr` — `kNoConversationId`, zero-cost.
   Endpoint composition: `ServerEndpoint::ConfigureXdlmsProcessor`
   и оба конструктора `GatewayEndpoint` ставят `&channel_` и на
   processor (`SetApduChannel`), и на декоратор
   (`SetCorrelationChannel`). End-to-end integration test
   `test/integration/test_endpoint_trace_correlation.cpp`
   (`dlms_endpoint_trace_correlation_tests`) гоняет один GET через
   `ServerEndpoint` и pin-ит, что один и тот же non-zero
   `conversationId` приходит на channel (`SetCorrelation` +
   `CorrelationAtSend`), оба xDLMS event-а (`RequestReceived` +
   `ResponseSent`) и единственный dispatch event (`GetDispatched`).
   §7 закрыт полностью. Полный ctest: 975/976 зелёные (тот же
   pre-existing `dlms_package_install_smoke` остаётся единственным
   красным, независимо от §7).

## P2. Надежность и оптимизация

1. Добавить fuzz/property tests для HDLC, LLC, Wrapper, BER/A-XDR decoders.
2. Добавить sanitizers в CI для поддерживаемых платформ.
3. Добавить microbenchmarks для codecs и profile frame paths.
4. Уменьшить allocation churn в APDU/xDLMS/profile paths.
5. Проверить overflow guards для всех size calculations.

## P2. Документация и сопровождение

1. Поддерживать `docs/system_architecture.md` как карту компонентов.
2. Поддерживать этот roadmap после каждого production-ready инкремента.
3. Для каждого публичного изменения обновлять:
   - `VERSION`;
   - `CHANGELOG.md`;
   - API docs;
   - `handoff.md`.

## Ближайший маршрут

1. P0: унифицировать small-buffer required-size contract для APDU raw xDLMS
   encode. Статус: выполнено в `0.3.16`.
2. P0: повторить тот же контракт для Wrapper encode. Статус: выполнено в
   `0.3.17`.
3. P0: повторить тот же контракт для HDLC encode, если strict encoder может
   вычислять полный размер до записи. Статус: выполнено в `0.3.18`.
4. P0: пройти C API output cleanup audit после required-size унификации.
   Статус: начато в `0.3.19` с Profile receive boundary validation.
5. P0: создать security support matrix и IC support matrix по базе знаний
   СПОДЭС/СПОДУС. Статус: выполнено в `docs/security_support_matrix.md` и
   `docs/ic_support_matrix.md`.
6. P0: реализовать минимальный `Security Setup` IC `64` surface с явными
   unsupported statuses для методов. Статус: выполнено в `0.3.20`.
7. P0: реализовать `security_activate` с monotonic policy strengthening.
   Статус: выполнено в `0.3.21`.
8. P0: закрепить refusal на local invocation counter exhaustion для protected
   APDU и HLS GMAC response paths. Статус: выполнено в `0.3.22`.
9. P0: добавить public invocation counter Data object
   `0.0.43.1.0.255`. Статус: выполнено в `0.3.23`.
10. P0: добавить per-sender replay state для protected APDU и HLS GMAC receive
    paths. Статус: выполнено в `0.3.24` для in-memory store; durable
    persistence остается application-provided.
11. P0: перейти к Security Setup key transfer/key wrapping contract или
   endpoint lifecycle cleanup tests после фиксации `Close()`
   semantics.
12. P0: довести `dlms-client` до удобного backend API для GUI-клиента вместо
    Gurux. Статус: detailed class-id/OBIS helpers добавлены в `0.6.0`; дальше
    common typed DLMS Data decode/encode helpers добавлены в `0.7.0`; дальше
    GUI-oriented read example добавлен в `0.7.1`; Profile Generic
    selector-specific helpers добавлены в `0.10.0`; DLMS `date-time`, `date`
    и `time` helpers добавлены в `0.12.0`; пример чтения типовых СПОДЭС OBIS
    добавлен в `0.12.1`; partial Clock `8` object добавлен в `0.13.0`.
