#include "dlms/llc/llc_c_api.h"

int dlms_llc_c_header_smoke(void)
{
  dlms_llc_header_t header;
  dlms_llc_lpdu_view_t lpdu;

  header.dsap = 0xE6u;
  header.ssap = 0xE6u;
  header.control = 0x00u;

  lpdu.header = header;
  lpdu.lsdu = 0;
  lpdu.lsdu_size = 0;

  return (int)(sizeof(header) + sizeof(lpdu));
}
