#!/usr/bin/env bash
# FROZEN — DO NOT MODIFY (see tests/FROZEN_MANIFEST.sha256)
# T-040 [integration] pluginval strictness 10 on the built VST3. Arg 1: CMake build dir.
# Env: PLUGINVAL (path to pluginval 1.0.4 binary, default: pluginval on PATH). Enforces: SC-1, SC-8, C-030.
set -euo pipefail
BUILD="${1:?build dir}"
PV="${PLUGINVAL:-pluginval}"
VST3=$(find "$BUILD" -type d -name "Refractor.vst3" -path "*VST3*" | head -n 1)
if [[ -z "$VST3" ]]; then echo "T-040 FAIL: Refractor.vst3 not found under $BUILD"; exit 1; fi
ARGS=(--strictness-level 10 --timeout-ms 600000 --random-seed 0x5eed --sample-rates 44100,48000,96000,192000 --block-sizes 1,64,128,512,1024 --output-dir "$BUILD/pluginval-logs")
if [[ "$(uname -s)" == "Linux" && -z "${DISPLAY:-}" ]]; then ARGS+=(--skip-gui-tests); fi
echo "T-040: $PV ${ARGS[*]} --validate $VST3"
"$PV" "${ARGS[@]}" --validate "$VST3"
