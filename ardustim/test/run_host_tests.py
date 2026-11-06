#!/usr/bin/env python3
"""Exercise F401 and ESP32-C3 firmware serial/storage paths with host mocks."""
from pathlib import Path
import os
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix="ardustim-test-") as directory:
  for target, flags in [("f401", ["-DARDUSTIM_F401", "-DSTM32F401xC"]),
                        ("esp32c3", ["-DARDUSTIM_ESP32C3", "-DCONFIG_IDF_TARGET_ESP32C3"])]:
    binary = Path(directory) / (target + "-test")
    subprocess.run([
        os.environ.get("CXX", "c++"), "-std=c++11", "-Wall", "-Wextra", "-Werror",
        "-fsanitize=undefined", "-fno-sanitize-recover=all",
        *flags,
        "-I" + str(root / "test/host"), "-I" + str(root / "ardustim"),
        str(root / "test/host/protocol_storage_test.cpp"), str(root / "ardustim/comms.cpp"),
        "-x", "c++", str(root / "ardustim/storage.ino"), "-o", str(binary),
    ], check=True)
    subprocess.run([str(binary)], check=True)
    backend_binary = Path(directory) / (target + "-backend-test")
    subprocess.run([
        os.environ.get("CXX", "c++"), "-std=c++11", "-Wall", "-Wextra", "-Werror",
        "-fsanitize=undefined", "-fno-sanitize-recover=all",
        *flags, "-DARDUSTIM_BACKEND_TEST",
        "-I" + str(root / "test/host"), "-I" + str(root / "ardustim"),
        str(root / "test/host/backend_test.cpp"), "-o", str(backend_binary),
    ], check=True)
    subprocess.run([str(backend_binary)], check=True)
