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
2. Завершить аудит очистки выходных структур C API.
   - Все выходные указатели очищаются до валидации.
   - Ошибки не оставляют старые значения в result/view/output структурах.
   - Profile receive boundary validation закрыта в `0.3.19`.
3. Закрепить status mapping между слоями.
   - Не сводить полезные ошибки к `InternalError`, если есть более точный
     публичный статус.
   - Документировать намеренные потери детализации на facade уровнях.
4. Проверить все C headers smoke tests.
   - Каждый C ABI header должен компилироваться как C.
   - Stable enum numeric checks должны быть только там, где ABI это требует.

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
     reply coverage добавлен в `0.3.29`; open-failure cleanup coverage
     требует продолжения по HLS GMAC negotiated paths.
3. Добавить regression tests для cleanup при неуспешном open/association.
4. Проверить, что bounded loops не скрывают `Timeout`, `Closed`, `InvalidState`.

## P0. Security и секреты

По базе знаний СПОДЭС/СПОДУС и DLMS/COSEM security не сводится к HLS GMAC:
нужны Security Setup IC `64`, suites `0/1/2`, управление ключами,
сертификатами, dedicated/general ciphering и строгая политика invocation
counter. Текущий код покрывает только часть Suite 0: low password,
HLS password/GMAC, AES-GCM ciphered APDU и локальные key/counter stores.
Текущий статус зафиксирован в `docs/security_support_matrix.md`.

1. Зафиксировать supported/unsupported security matrix.
   - Suite 0: `None`, Low, High password, High GMAC, global ciphering.
   - Suite 0 gaps: Security Setup IC `64`, `security_activate`,
     `global_key_transfer`, `change_HLS_secret`, dedicated key и dedicated
     ciphering APDU.
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
     policy strengthening; key/certificate/key agreement semantics остаются
     следующими задачами.
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
     test-only diagnostic build.
   - Add redaction tests for endpoint/client live-smoke diagnostics.
   - Document storage, ownership and lifetime for key stores and invocation
     counter stores.

## P0. COSEM IC и СПОДЭС/СПОДУС model coverage

База знаний указывает, что для СПОДЭС/СПОДУС одних protocol services
недостаточно. Нужен прикладной объектный слой с обязательными IC, OBIS
каталогами, access rights, Profile Generic структурами и наборами параметров.
Текущий `dlms-cosem` реализует минимальный generic object model и часть
Data/Register/Association LN/SAP Assignment behavior, но не является полной
СПОДЭС/СПОДУС моделью.
Текущий статус зафиксирован в `docs/ic_support_matrix.md`.

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
4. СПОДЭС/СПОДУС catalogs.
   - OBIS catalogs and parameter lists for meter categories A/B/C/D and ИВКЭ.
   - Event code table and status word formats.
   - Access policy profiles for public reader/configurator modes.
5. Association LN object list and access rights.
   - Object list must reflect visible COSEM objects and access rights for the
     current association/security context.
   - `reply_to_HLS_authentication`, object add/remove where supported,
     association status, xDLMS context info.
6. Push setup and initiative messages.
   - Push Setup IC `40` version handling.
   - Notification payload structures required by СПОДЭС/СПОДУС.
   - Rule: transmit only actual data not yet sent/confirmed where this is part
     of ИВКЭ behavior.
7. Image Transfer and control classes.
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
   - Уже есть `codec`, `protocol`, `runtime`.
   - Добавить `io`, `cosem_server`, `framework` consumers.
2. Продолжать exported target audit.
   - Include dirs, transitive deps, OpenSSL dependency, отсутствие test deps.
3. Документировать минимальные include примеры для каждого aggregate target.
4. Проверить package artifact на Windows/MSYS2 и Linux CI, если Linux runner
   доступен.

## P1. Диагностика

1. Упорядочить trace contracts по transport/profile/association/endpoint.
2. Добавить correlation metadata для multi-layer traces без раскрытия секретов.
3. Проверить status-to-string полноту во всех публичных status enum.
4. Сделать live smoke output пригодным для поддержки, но безопасным.

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
