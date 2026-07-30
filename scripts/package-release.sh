#!/usr/bin/env bash
set -Eeuo pipefail

readonly ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
readonly VERSION="1.0.0"
readonly NAME="windtrail-${VERSION}"
readonly OUT_DIR="${1:-${ROOT_DIR}/dist}"

mkdir -p "$OUT_DIR"
work_dir="$(mktemp -d)"
trap 'rm -rf "$work_dir"' EXIT

rsync -a --delete \
  --exclude build \
  --exclude dist \
  --exclude .git \
  "$ROOT_DIR/" "$work_dir/$NAME/"

find "$work_dir/$NAME" -type f -exec touch -d '2026-07-30 17:45:00 -0300' {} +

(
  cd "$work_dir"
  zip -qr "$OUT_DIR/${NAME}.zip" "$NAME"
  tar -czf "$OUT_DIR/${NAME}.tar.gz" "$NAME"
)

(
  cd "$OUT_DIR"
  sha256sum "${NAME}.zip" "${NAME}.tar.gz" > "${NAME}-SHA256SUMS.txt"
)

printf 'Created:
  %s
  %s
  %s
' \
  "$OUT_DIR/${NAME}.zip" \
  "$OUT_DIR/${NAME}.tar.gz" \
  "$OUT_DIR/${NAME}-SHA256SUMS.txt"
