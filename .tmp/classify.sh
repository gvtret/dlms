#!/usr/bin/env bash
set -u
cd /e/work/dlms
files=(
  CHANGELOG.md
  VERSION
  docs/ic_support_matrix.md
  lib/dlms-cosem/docs/01_cosem_api.md
  lib/dlms-cosem/docs/03_cosem_test_plan.md
)
for f in "${files[@]}"; do
  echo "=== $f ==="
  push=$(git diff -- "$f" | grep -cE 'Push|push_setup|push_object|push_client|push_protect|push_oper|confirmation_param|last_confirmation|repetition_delay|port_reference')
  sfsk=$(git diff -- "$f" | grep -cE 'S-FSK|SFsk|s_fsk|search_initiator|mark_freq|space_freq|mac_group_addr|repeater_status|initiator_mac|synchronization_locked|transmission_speed')
  echo "  push: $push"
  echo "  sfsk: $sfsk"
done
