#!/bin/bash
set -e

QT_HOST=${QT_HOST:-$HOME/.local/android-toolchain/qt/6.7.3/gcc_64}
QT_ANDROID=${QT_ANDROID:-$HOME/.local/android-toolchain/qt/6.7.3/android_arm64_v8a}
ANDROID_SDK_ROOT=${ANDROID_SDK_ROOT:-$HOME/.local/android-toolchain/sdk}
ANDROID_NDK_ROOT=${ANDROID_NDK_ROOT:-$ANDROID_SDK_ROOT/ndk/26.1.10909125}
JAVA_HOME=${JAVA_HOME:-$HOME/.local/android-toolchain/jdk}
GRADLE_BIN=${GRADLE_BIN:-$HOME/.local/android-toolchain/gradle-8.3/bin}

export JAVA_HOME ANDROID_SDK_ROOT ANDROID_NDK_ROOT
[ -n "$GRADLE_BIN" ] && export PATH="$GRADLE_BIN:$PATH"

SRC_DIR=$(cd "$(dirname "$0")/.." && pwd)
BUILD_DIR=${BUILD_DIR:-${XDG_CACHE_HOME:-$HOME/.cache}/butas-build-apk}

rm -rf "$BUILD_DIR"
cmake -S "$SRC_DIR" -B "$BUILD_DIR" \
  -DCMAKE_TOOLCHAIN_FILE="$QT_ANDROID/lib/cmake/Qt6/qt.toolchain.cmake" \
  -DQT_HOST_PATH="$QT_HOST" \
  -DANDROID_SDK_ROOT="$ANDROID_SDK_ROOT" \
  -DANDROID_NDK_ROOT="$ANDROID_NDK_ROOT" \
  -DANDROID_ABI=arm64-v8a \
  -DQT_ANDROID_BUILD_ALL_ABIS=OFF \
  -DCMAKE_BUILD_TYPE=Release

cmake --build "$BUILD_DIR" --target apk

echo
echo "Imzasiz APK: $BUILD_DIR/android-build/build/outputs/apk/release/android-build-release-unsigned.apk"
