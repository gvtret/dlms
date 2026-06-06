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
2. Проверить ошибки при частично открытых transport/profile/association цепочках.
3. Добавить regression tests для cleanup при неуспешном open/association.
4. Проверить, что bounded loops не скрывают `Timeout`, `Closed`, `InvalidState`.

## P0. Security и секреты

1. Проверить, что trace/log paths не выводят ключи, password, challenges и GMAC
   material.
2. Добавить тесты на redaction для endpoint/client live-smoke diagnostics.
3. Проверить HLS GMAC и ciphered APDU vectors на корректность invocation
   counter handling.
4. Описать требования к storage, ownership и lifetime для key stores и
   invocation counter stores.

## P0. Протокольная полнота MVP

1. Association lifecycle: open, release, abort, low password, HLS GMAC.
2. xDLMS GET/SET/ACTION: normal и block transfer в обе стороны.
3. Server path: COSEM object lookup, access rights, association view, logical
   device name, SAP assignment.
4. Profile path: Wrapper/TCP, Wrapper/UDP, HDLC over byte stream, HDLC session.
5. Client facade и endpoint facade: сценарии client, server, push listener,
   gateway.

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
5. P0: перейти к endpoint lifecycle cleanup tests.
