#!/bin/bash
cd /e/work/dlms
grep -nE '^class Cosem.*Object : public ICosemObject' lib/dlms-cosem/include/dlms/cosem/simple_objects.hpp \
  | sed 's/.*class \(Cosem[A-Za-z0-9]*Object\).*/\1/' \
  | while read cls; do
      # extract class_id from the cpp constants
      id=$(grep -E "constexpr std::uint16_t k.*ClassId = [0-9]+u;" lib/dlms-cosem/src/cosem/simple_objects.cpp \
           | head -100 | grep -B0 "" || true)
      echo "$cls"
    done
