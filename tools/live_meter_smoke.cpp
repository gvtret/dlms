#include "dlms/client/client.hpp"
#include "dlms/xdlms/xdlms_client.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {

const char* Env(const char* name)
{
  return std::getenv(name);
}

bool ParseUnsigned(
  const char* text,
  unsigned long maximum,
  unsigned long& output)
{
  if (text == 0 || text[0] == '\0') {
    return false;
  }

  char* end = 0;
  const unsigned long value = std::strtoul(text, &end, 10);
  if (end == text || *end != '\0' || value > maximum) {
    return false;
  }

  output = value;
  return true;
}

unsigned long EnvUnsigned(
  const char* name,
  unsigned long defaultValue,
  unsigned long maximum,
  bool& ok)
{
  const char* value = Env(name);
  if (value == 0 || value[0] == '\0') {
    return defaultValue;
  }

  unsigned long parsed = 0;
  if (!ParseUnsigned(value, maximum, parsed)) {
    std::cerr << "config " << name << " invalid: " << value << "\n";
    ok = false;
    return defaultValue;
  }

  return parsed;
}

int HexValue(char value)
{
  if (value >= '0' && value <= '9') {
    return value - '0';
  }
  if (value >= 'a' && value <= 'f') {
    return 10 + value - 'a';
  }
  if (value >= 'A' && value <= 'F') {
    return 10 + value - 'A';
  }
  return -1;
}

bool EnvHexBytes(
  const char* name,
  std::uint8_t* output,
  std::size_t outputSize)
{
  const char* value = Env(name);
  if (value == 0 || std::strlen(value) != outputSize * 2u) {
    std::cerr << "config " << name << " requires "
              << (outputSize * 2u) << " hex chars\n";
    return false;
  }

  for (std::size_t i = 0u; i < outputSize; ++i) {
    const int high = HexValue(value[i * 2u]);
    const int low = HexValue(value[i * 2u + 1u]);
    if (high < 0 || low < 0) {
      std::cerr << "config " << name << " invalid hex\n";
      return false;
    }
    output[i] = static_cast<std::uint8_t>((high << 4) | low);
  }
  return true;
}

bool EnvHexBytesOptional(
  const char* name,
  std::uint8_t* output,
  std::size_t outputSize)
{
  const char* value = Env(name);
  if (value == 0 || value[0] == '\0') {
    return true;
  }

  return EnvHexBytes(name, output, outputSize);
}

bool EnvAuthentication(
  dlms::client::DlmsClientOptions& options,
  bool& ok)
{
  const char* mode = Env("DLMS_LIVE_AUTHENTICATION");
  if (mode == 0 || mode[0] == '\0' || std::strcmp(mode, "none") == 0) {
    options.authenticationMode =
      dlms::client::ClientAuthenticationMode::None;
    return true;
  }

  if (std::strcmp(mode, "lls") == 0) {
    const char* password = Env("DLMS_LIVE_LLS_PASSWORD");
    if (password == 0 || password[0] == '\0') {
      std::cerr << "config DLMS_LIVE_LLS_PASSWORD required for lls\n";
      return false;
    }

    options.authenticationMode =
      dlms::client::ClientAuthenticationMode::LowLevelSecurity;
    options.lowLevelSecurity.credential =
      reinterpret_cast<const std::uint8_t*>(password);
    options.lowLevelSecurity.credentialSize = std::strlen(password);
    return true;
  }

  if (std::strcmp(mode, "high") == 0) {
    const char* password = Env("DLMS_LIVE_HLS_PASSWORD");
    if (password == 0 || password[0] == '\0') {
      std::cerr << "config DLMS_LIVE_HLS_PASSWORD required for high\n";
      return false;
    }

    options.authenticationMode =
      dlms::client::ClientAuthenticationMode::HighLevelSecurity;
    options.highLevelSecurity.password =
      reinterpret_cast<const std::uint8_t*>(password);
    options.highLevelSecurity.passwordSize = std::strlen(password);
    return true;
  }

  if (std::strcmp(mode, "hls-gmac") != 0) {
    std::cerr << "config DLMS_LIVE_AUTHENTICATION invalid: " << mode << "\n";
    return false;
  }

  options.authenticationMode =
    dlms::client::ClientAuthenticationMode::HighLevelSecurityGmac;
  options.security.invocationCounter = static_cast<std::uint32_t>(
    EnvUnsigned(
      "DLMS_LIVE_INVOCATION_COUNTER",
      1u,
      std::numeric_limits<std::uint32_t>::max(),
      ok));

  if (!EnvHexBytes(
        "DLMS_LIVE_CLIENT_SYSTEM_TITLE_HEX",
        options.security.clientSystemTitle,
        sizeof(options.security.clientSystemTitle)) ||
      !EnvHexBytesOptional(
        "DLMS_LIVE_SERVER_SYSTEM_TITLE_HEX",
        options.security.serverSystemTitle,
        sizeof(options.security.serverSystemTitle)) ||
      !EnvHexBytes(
        "DLMS_LIVE_AUTHENTICATION_KEY_HEX",
        options.security.authenticationKey,
        sizeof(options.security.authenticationKey))) {
    return false;
  }

  return true;
}

bool EnvProfile(dlms::client::DlmsClientOptions& options)
{
  const char* profile = Env("DLMS_LIVE_PROFILE");
  if (profile == 0 || profile[0] == '\0' ||
      std::strcmp(profile, "wrapper-tcp") == 0) {
    options.profile = dlms::client::ClientProfile::WrapperTcp;
    return true;
  }

  if (std::strcmp(profile, "hdlc-tcp") == 0) {
    options.profile = dlms::client::ClientProfile::HdlcTcp;
    return true;
  }

  std::cerr << "config DLMS_LIVE_PROFILE invalid: " << profile << "\n";
  return false;
}

bool EnvTraceEnabled()
{
  const char* trace = Env("DLMS_LIVE_TRACE");
  return trace != 0 && std::strcmp(trace, "1") == 0;
}

const char* ProfileName(dlms::client::ClientProfile profile)
{
  switch (profile) {
  case dlms::client::ClientProfile::WrapperTcp:
    return "wrapper-tcp";
  case dlms::client::ClientProfile::HdlcTcp:
    return "hdlc-tcp";
  }
  return "unknown";
}

const char* AuthenticationName(
  dlms::client::ClientAuthenticationMode mode)
{
  switch (mode) {
  case dlms::client::ClientAuthenticationMode::None:
    return "none";
  case dlms::client::ClientAuthenticationMode::LowLevelSecurity:
    return "lls";
  case dlms::client::ClientAuthenticationMode::HighLevelSecurity:
    return "high";
  case dlms::client::ClientAuthenticationMode::HighLevelSecurityGmac:
    return "hls-gmac";
  }
  return "unknown";
}

void PrintObis(const dlms::xdlms::CosemLogicalName& name)
{
  for (std::size_t i = 0u; i < name.Size(); ++i) {
    if (i != 0u) {
      std::cout << ".";
    }
    std::cout << static_cast<unsigned>(name[i]);
  }
}

bool ParseObis(const char* text, dlms::xdlms::CosemLogicalName& output)
{
  if (text == 0 || text[0] == '\0') {
    return false;
  }

  std::vector<unsigned long> parts;
  std::stringstream stream(text);
  std::string item;
  while (std::getline(stream, item, '.')) {
    unsigned long value = 0;
    if (!ParseUnsigned(item.c_str(), 255u, value)) {
      return false;
    }
    parts.push_back(value);
  }

  if (parts.size() != 6u) {
    return false;
  }

  output = dlms::xdlms::CosemLogicalName(
    static_cast<std::uint8_t>(parts[0]),
    static_cast<std::uint8_t>(parts[1]),
    static_cast<std::uint8_t>(parts[2]),
    static_cast<std::uint8_t>(parts[3]),
    static_cast<std::uint8_t>(parts[4]),
    static_cast<std::uint8_t>(parts[5]));
  return true;
}

dlms::client::CosemAttributeDescriptor MakeDescriptor(bool& ok)
{
  dlms::client::CosemAttributeDescriptor descriptor =
    dlms::xdlms::EmptyCosemAttributeDescriptor();

  descriptor.instanceId = dlms::xdlms::CosemLogicalName(0, 0, 42, 0, 0, 255);
  const char* obis = Env("DLMS_LIVE_OBIS");
  if (obis != 0 && obis[0] != '\0' &&
      !ParseObis(obis, descriptor.instanceId)) {
    std::cerr << "config DLMS_LIVE_OBIS invalid: " << obis << "\n";
    ok = false;
  }

  descriptor.classId = static_cast<std::uint16_t>(
    EnvUnsigned("DLMS_LIVE_CLASS_ID", 1u, 0xffffu, ok));
  descriptor.attributeId = static_cast<std::uint8_t>(
    EnvUnsigned("DLMS_LIVE_ATTRIBUTE_ID", 2u, 0xffu, ok));
  return descriptor;
}

dlms::client::DlmsClientOptions MakeOptions(
  const char* host,
  bool& ok)
{
  dlms::client::DlmsClientOptions options =
    dlms::client::DefaultDlmsClientOptions();

  if (!EnvProfile(options)) {
    ok = false;
  }

  options.wrapperTcp.host = host;
  options.wrapperTcp.port = static_cast<std::uint16_t>(
    EnvUnsigned("DLMS_LIVE_WRAPPER_PORT", 4059u, 0xffffu, ok));
  options.clientSap = static_cast<std::uint16_t>(
    EnvUnsigned("DLMS_LIVE_CLIENT_SAP", 16u, 0xffffu, ok));
  options.serverSap = static_cast<std::uint16_t>(
    EnvUnsigned("DLMS_LIVE_SERVER_SAP", 1u, 0xffffu, ok));
  options.wrapperTcp.sourceWPort = static_cast<std::uint16_t>(
    EnvUnsigned(
      "DLMS_LIVE_SOURCE_WPORT",
      options.clientSap,
      0xffffu,
      ok));
  options.wrapperTcp.destinationWPort = static_cast<std::uint16_t>(
    EnvUnsigned(
      "DLMS_LIVE_DEST_WPORT",
      options.serverSap,
      0xffffu,
      ok));
  options.hdlcTcp.host = host;
  options.hdlcTcp.port = options.wrapperTcp.port;
  options.hdlcTcp.clientAddress = static_cast<std::uint8_t>(
    EnvUnsigned(
      "DLMS_LIVE_HDLC_CLIENT_ADDRESS",
      options.clientSap,
      0x7fu,
      ok));
  options.hdlcTcp.logicalDeviceAddress = static_cast<std::uint16_t>(
    EnvUnsigned(
      "DLMS_LIVE_HDLC_LOGICAL_DEVICE_ADDRESS",
      options.serverSap,
      0x3fffu,
      ok));
  options.hdlcTcp.physicalDeviceAddress = static_cast<std::uint16_t>(
    EnvUnsigned(
      "DLMS_LIVE_HDLC_PHYSICAL_DEVICE_ADDRESS",
      0u,
      0x3fffu,
      ok));
  options.hdlcTcp.maxInfoTx = static_cast<std::size_t>(
    EnvUnsigned("DLMS_LIVE_HDLC_MAX_INFO_TX", 128u, 0xffffu, ok));
  options.hdlcTcp.maxInfoRx = static_cast<std::size_t>(
    EnvUnsigned("DLMS_LIVE_HDLC_MAX_INFO_RX", 128u, 0xffffu, ok));
  options.hdlcTcp.windowSizeTx = static_cast<std::uint8_t>(
    EnvUnsigned("DLMS_LIVE_HDLC_WINDOW_TX", 1u, 0xffu, ok));
  options.hdlcTcp.windowSizeRx = static_cast<std::uint8_t>(
    EnvUnsigned("DLMS_LIVE_HDLC_WINDOW_RX", 1u, 0xffu, ok));
  options.hdlcTcp.retryCount = static_cast<std::uint8_t>(
    EnvUnsigned("DLMS_LIVE_HDLC_RETRY_COUNT", 3u, 0xffu, ok));
  options.hdlcTcp.retryDelayMs = static_cast<std::uint32_t>(
    EnvUnsigned(
      "DLMS_LIVE_HDLC_RETRY_DELAY_MS",
      10u,
      std::numeric_limits<std::uint32_t>::max(),
      ok));
  options.connectTimeoutMs = static_cast<std::uint32_t>(
    EnvUnsigned(
      "DLMS_LIVE_CONNECT_TIMEOUT_MS",
      5000u,
      std::numeric_limits<std::uint32_t>::max(),
      ok));
  options.requestTimeoutMs = static_cast<std::uint32_t>(
    EnvUnsigned(
      "DLMS_LIVE_REQUEST_TIMEOUT_MS",
      5000u,
      std::numeric_limits<std::uint32_t>::max(),
      ok));
  if (!EnvAuthentication(options, ok)) {
    ok = false;
  }

  return options;
}

void PrintTrace(
  const dlms::client::DlmsClientOptions& options,
  const dlms::client::CosemAttributeDescriptor& descriptor)
{
  std::cout << "trace: profile=" << ProfileName(options.profile)
            << " endpoint=" << options.wrapperTcp.host
            << ":" << options.wrapperTcp.port
            << " clientSap=" << options.clientSap
            << " serverSap=" << options.serverSap
            << " authentication="
            << AuthenticationName(options.authenticationMode)
            << "\n";
  std::cout << "trace: wrapper sourceWPort="
            << options.wrapperTcp.sourceWPort
            << " destWPort=" << options.wrapperTcp.destinationWPort
            << "\n";
  std::cout << "trace: hdlc clientAddress="
            << static_cast<unsigned>(options.hdlcTcp.clientAddress)
            << " logicalDeviceAddress="
            << options.hdlcTcp.logicalDeviceAddress
            << " physicalDeviceAddress="
            << options.hdlcTcp.physicalDeviceAddress
            << " maxInfoTx=" << options.hdlcTcp.maxInfoTx
            << " maxInfoRx=" << options.hdlcTcp.maxInfoRx
            << " windowTx="
            << static_cast<unsigned>(options.hdlcTcp.windowSizeTx)
            << " windowRx="
            << static_cast<unsigned>(options.hdlcTcp.windowSizeRx)
            << " retryCount="
            << static_cast<unsigned>(options.hdlcTcp.retryCount)
            << " retryDelayMs=" << options.hdlcTcp.retryDelayMs
            << "\n";
  std::cout << "trace: get classId=" << descriptor.classId
            << " obis=";
  PrintObis(descriptor.instanceId);
  std::cout << " attributeId="
            << static_cast<unsigned>(descriptor.attributeId)
            << "\n";
}

int Fail(const char* step, dlms::client::ClientStatus status)
{
  std::cerr << step << ": "
            << dlms::client::ClientStatusName(status) << "\n";
  return 1;
}

} // namespace

int main()
{
  const char* host = Env("DLMS_LIVE_WRAPPER_HOST");
  if (host == 0 || host[0] == '\0') {
    std::cout << "LiveMeterSmoke: skipped, DLMS_LIVE_WRAPPER_HOST is not set\n";
    return 0;
  }

  bool ok = true;
  const dlms::client::DlmsClientOptions options = MakeOptions(host, ok);
  const dlms::client::CosemAttributeDescriptor descriptor =
    MakeDescriptor(ok);
  if (!ok) {
    return 1;
  }

  if (EnvTraceEnabled()) {
    PrintTrace(options, descriptor);
  }

  dlms::client::DlmsClient client(options);
  dlms::client::ClientStatus status = client.Connect();
  if (status != dlms::client::ClientStatus::Ok) {
    return Fail("connect", status);
  }
  std::cout << "connect: Ok\n";

  status = client.OpenAssociation();
  if (status != dlms::client::ClientStatus::Ok) {
    client.Close();
    return Fail("association", status);
  }
  std::cout << "association: Ok\n";

  std::vector<std::uint8_t> data;
  status = client.Get(descriptor, data);
  if (status != dlms::client::ClientStatus::Ok) {
    client.Close();
    return Fail("get", status);
  }
  std::cout << "get: Ok bytes=" << data.size() << "\n";

  status = client.ReleaseAssociation();
  if (status != dlms::client::ClientStatus::Ok) {
    std::cout << "release: "
              << dlms::client::ClientStatusName(status)
              << " (close fallback)\n";
  } else {
    std::cout << "release: Ok\n";
  }

  status = client.Close();
  if (status != dlms::client::ClientStatus::Ok) {
    return Fail("close", status);
  }
  std::cout << "close: Ok\n";
  return 0;
}
