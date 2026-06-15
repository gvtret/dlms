import re
p = r'E:\work\dlms\lib\dlms-cosem\test\cosem\test_simple_objects.cpp'
s = open(p, encoding='utf-8').read()

# Block to remove: from "namespace {\n\ndlms::cosem::CosemByteBuffer SamplePrimePlcMacAddress()"
# up to (but NOT including) the closing "namespace {" line that begins the next
# helper (PrimePlcApplicationIdentifier).
start_marker = "namespace {\n\ndlms::cosem::CosemByteBuffer SamplePrimePlcMacAddress()"
end_marker = "namespace {\n\ndlms::cosem::CosemByteBuffer SamplePrimePlcApplicationIdentifier()"

i = s.index(start_marker)
j = s.index(end_marker)
removed = s[i:j]
print(f"removing {len(removed)} bytes from offset {i} to {j}")
# sanity: ensure the removed region only references PrimePlcMacAddressSetup-related symbols
assert "PrimePlcApplicationIdentification" not in removed, "would remove too much"
assert "PrimePlcMacAddressSetup" in removed
new = s[:i] + s[j:]
open(p, 'w', encoding='utf-8', newline='\n').write(new)
print("done")
