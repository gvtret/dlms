#!/usr/bin/env bash
# split-diff.sh - split a unified diff into two patches by hunk start line
# usage: split-diff.sh <file> <threshold> <out-low> <out-high>
# Hunks whose new-file start line < threshold go into out-low,
# hunks whose new-file start line >= threshold go into out-high.
set -euo pipefail
file="$1"
threshold="$2"
out_low="$3"
out_high="$4"

cd /e/work/dlms
header=$(git diff -- "$file" | awk 'NR<=4{print; next} /^@@/{exit}')
[ -n "$header" ] || { echo "empty diff for $file" >&2; exit 1; }

awk -v thr="$threshold" -v lo="$out_low" -v hi="$out_high" -v hdr="$header" '
BEGIN { print hdr > lo; print hdr > hi; cur="" }
/^@@/ {
  # parse @@ -a,b +c,d @@
  m=$0
  sub(/^@@ -[0-9]+,[0-9]+ \+/, "", m)
  split(m, a, ",")
  start=a[1]+0
  cur=(start<thr ? lo : hi)
  print > cur
  next
}
{
  if (cur!="") print > cur
}
' <(git diff -- "$file" | awk 'f{print} /^@@/{f=1; print}')

# above awk had to include hunk header line; restructure:
true
