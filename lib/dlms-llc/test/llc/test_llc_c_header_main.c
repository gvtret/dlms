/* C-only smoke driver: ensures the C ABI links from a pure-C TU and the
   declared smoke function is actually called at runtime.  The function's
   return contract is pinned by its corresponding C++ gtest case; here we
   only require the call to succeed without re-encoding that contract. */

extern int dlms_llc_c_header_smoke(void);

int main(void) { (void)dlms_llc_c_header_smoke(); return 0; }
