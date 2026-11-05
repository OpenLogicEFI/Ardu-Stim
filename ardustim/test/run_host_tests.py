#!/usr/bin/env python3
"""Exercise firmware serial/storage paths and the real GUI packet sender."""
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
        str(root / "test/host/f401_test.cpp"), str(root / "ardustim/comms.cpp"),
        "-x", "c++", str(root / "ardustim/storage.ino"), "-o", str(binary),
    ], check=True)
    subprocess.run([str(binary)], check=True)

with tempfile.TemporaryDirectory(prefix="ardustim-avr-storage-") as directory:
    binary = Path(directory) / "storage-test"
    subprocess.run([
        os.environ.get("CXX", "c++"), "-std=c++11", "-Wall", "-Wextra", "-Werror",
        "-fsanitize=undefined", "-fno-sanitize-recover=all",
        "-I" + str(root / "test/host"), "-I" + str(root / "ardustim"),
        "-D__AVR__", str(root / "test/host/avr_storage_test.cpp"),
        "-x", "c++", str(root / "ardustim/storage.ino"), "-o", str(binary),
    ], check=True)
    subprocess.run([str(binary)], check=True)
subprocess.run(["node", str(root / "test/check_gui_packet.js")], check=True)
