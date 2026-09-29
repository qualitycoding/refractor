# Environment (pinned; Linux setup verified in the planning sandbox 2026-09-28, see research/spikes/S2-env)

## Lock table
| Component | Version | Pin | Where |
|---|---|---|---|
| C++ standard | C++20 | `CMAKE_CXX_STANDARD 20` | CMakeLists.txt |
| JUCE | 8.0.15 | `91ad83ae34a81e0833b1a2b0866f54846370ae53` | cmake/Dependencies.cmake |
| Catch2 | v3.16.0 | `317ac1ed4c0bb6e6b91eafc817e05c488feffcb3` | cmake/Dependencies.cmake |
| CMake | 3.31.10 | `pip install cmake==3.31.10` | all OSes |
| Ninja | 1.13.0 | `pip install ninja==1.13.0` | all OSes |
| Python | ≥3.10 (3.12.3 verified) | runner/system | scripts |
| GCC (Linux) | 13.3.0 (Ubuntu 24.04 default) | apt | verified |
| Xcode (macOS) | runner default on `macos-15` | image | CI only (not verifiable in sandbox, R-009) |
| MSVC (Windows) | VS 2022 on `windows-2022` | image | CI only (R-009) |
| pluginval | v1.0.4 | see §Pinned artefacts | tests/scripts/run_pluginval.sh |

## Pinned artefacts (sha256)
| File | URL | sha256 |
|---|---|---|
| pluginval_Linux.zip | https://github.com/Tracktion/pluginval/releases/download/v1.0.4/pluginval_Linux.zip | `c01c49d8063965c4c2dea8324468336768f5c9139e0b1caebde14c2400b55352` |
| pluginval_macOS.zip | https://github.com/Tracktion/pluginval/releases/download/v1.0.4/pluginval_macOS.zip | `3c4c533bda0c5059eea3ddaea752d757ee2025041f0f47e6bcb0e87f6082b29f` |
| pluginval_Windows.zip | https://github.com/Tracktion/pluginval/releases/download/v1.0.4/pluginval_Windows.zip | `c08e61ce3b96db41636f8ec7e76f4c7e2c13ebdac7fa1b5a1f52b4f32ec715ab` |

## §Linux setup (Ubuntu 24.04; executed successfully in sandbox)
```bash
sudo apt-get update
sudo apt-get install -y g++ git python3-pip xvfb \
  libasound2-dev libfreetype-dev libfontconfig1-dev libcurl4-openssl-dev \
  libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libxrender-dev \
  libglu1-mesa-dev mesa-common-dev
python3 -m pip install --user cmake==3.31.10 ninja==1.13.0   # add --break-system-packages on PEP-668 systems
export PATH="$HOME/.local/bin:$PATH"
curl -fsSL -o /tmp/pluginval.zip https://github.com/Tracktion/pluginval/releases/download/v1.0.4/pluginval_Linux.zip
echo "c01c49d8063965c4c2dea8324468336768f5c9139e0b1caebde14c2400b55352  /tmp/pluginval.zip" | sha256sum -c -
unzip -o /tmp/pluginval.zip -d "$HOME/.local/bin" && export PLUGINVAL="$HOME/.local/bin/pluginval"
```
Build & test:
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_BASE_DIR=$PWD/.fc
cmake --build build -j"$(nproc)"
xvfb-run -a ctest --test-dir build -E '^perf$' --output-on-failure
ctest --test-dir build -L perf --output-on-failure
```
Sanitizer configuration (T-052):
```bash
cmake -S . -B build-asan -G Ninja -DCMAKE_BUILD_TYPE=Debug -DREFRACTOR_SANITIZE=ON -DREFRACTOR_BUILD_PLUGIN=OFF -DFETCHCONTENT_BASE_DIR=$PWD/.fc
cmake --build build-asan && ctest --test-dir build-asan -R '^(dsp_tests|rt_tests)$' --output-on-failure
```
Note: ASan replaces operator new; `rt_tests` defines its own global operator new — if ASan reports an ODR/interposition
conflict, CI runs only `dsp_tests` under sanitizers and logs it in DEVIATIONS.md (R-013).

## §macOS / §Windows setup
Same cmake/ninja pins via `python3 -m pip install cmake==3.31.10 ninja==1.13.0`. macOS configure adds
`-DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY=-`. Windows (`windows-2022`, VS 2022 preinstalled) uses the multi-config generator: `cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DFETCHCONTENT_BASE_DIR=%CD%/.fc`, `cmake --build build --config Release`, `ctest --test-dir build -C Release -E "^perf$" --output-on-failure` (run from Git Bash so `run_pluginval.sh` works).

## §CI (GitHub Actions pins)
| Action | Tag | SHA |
|---|---|---|
| actions/checkout | v7.0.1 | `3d3c42e5aac5ba805825da76410c181273ba90b1` |
| actions/setup-python | v7.0.0 | `5fda3b95a4ea91299a34e894583c3862153e4b97` |
| actions/upload-artifact | v7.0.1 | `043fb46d1a93c77aae656e7c1c64a875d1fc6a0a` |
Runner labels: `ubuntu-24.04`, `macos-15`, `windows-2022` (C-033). CI uses only `GITHUB_TOKEN` (no repo secrets).

## §Credentials
| Name | Scope | Used by | Notes |
|---|---|---|---|
| `GH_TOKEN` | fine-grained PAT, repo `qualitycoding/refractor`: Contents RW, Pull requests RW, Actions R | implementer (push, PR, CI polling) | Supplied by owner; never committed. The planning PAT must be revoked by the owner after planning (R-014). |
| `GITHUB_TOKEN` | automatic, read-only contents | CI | default permissions `contents: read` set in workflow. |
No staging/sandbox targets exist (`software.deploys=false`).
