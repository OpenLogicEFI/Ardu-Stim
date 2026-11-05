#!/usr/bin/env python3
"""Exercise F401 calculations and actual serial/storage code with host mocks."""
from pathlib import Path
import os
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix="ardustim-test-") as directory:
    binary = Path(directory) / "f401-test"
    subprocess.run([
        os.environ.get("CXX", "c++"), "-std=c++11", "-Wall", "-Wextra", "-Werror",
        "-fsanitize=undefined", "-fno-sanitize-recover=all",
        "-DARDUSTIM_F401", "-DSTM32F401xC",
        "-I" + str(root / "test/host"), "-I" + str(root / "ardustim"),
        str(root / "test/host/f401_test.cpp"), str(root / "ardustim/comms.cpp"),
        "-x", "c++", str(root / "ardustim/storage.ino"), "-o", str(binary),
    ], check=True)
    subprocess.run([str(binary)], check=True)
