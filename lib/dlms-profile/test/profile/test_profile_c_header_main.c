/* C-only smoke driver: ensures the C ABI links from a pure-C TU and the
   declared smoke function is actually called at runtime.  The function's
   return contract is pinned by its corresponding C++ gtest case; here we
   only require the call to succeed without re-encoding that contract. */

extern int dlms_profile_c_header_compiles(void);

int main(void) { (void)dlms_profile_c_header_compiles(); return 0; }
