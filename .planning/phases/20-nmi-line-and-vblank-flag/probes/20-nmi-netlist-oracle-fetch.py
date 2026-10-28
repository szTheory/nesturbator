#!/usr/bin/env python3
"""Reproduce the research-only die-netlist oracle without vendoring its inputs.

Run from any directory with Python 3 and Node available. Published simulator
files stay in memory and must match the recorded SHA-256 values before use.
This command does not run the NES core or certify a paired-chip timing claim.
"""

import hashlib
import json
from pathlib import Path
import subprocess
import sys
import urllib.request


BASE = "https://www.qmtpro.com/~nes/chipimages/visual2a03/"
EXPECTED = {
    "segdefs.js": "fe34a098ec64ee9049a0e083ec337e4d5228f0d338e1c64859c6bf2b1b2e7197",
    "transdefs.js": "b10ce2c7c5b9774bb4f27476cb3e44a1ba1b290e8a463df6957803950cf44607",
    "nodenames.js": "e88eeec604234012a82beb172145426c2391a06cf618c8a5f8ab32c872840679",
    "wires.js": "dded17b4e9e0752a450e1f684925c2bf67ece24977006a75eaaa43325b5d6387",
    "chipsim.js": "3fe3fa48a003704ef66346e042adb016c0b3bbee018e267edb4ef921ba9cc2a1",
    "macros.js": "c8ec996fbad9434f84b4b829f20797df0cceae0828d87e298ca28352dfd1e8e5",
}
DRIVER_HASH = "9643f6f1d6574ac82250f2d0fd34f56db87b4de4efd39e900070a231aa5299d2"
OUTPUT_HASH = "5d1abd0c34ccd0aafd47988a74bcd5dfdeb11c8841e37f22e7e87fa6d3a02bc1"


def main():
    driver = Path(__file__).with_name("20-nmi-netlist-oracle.cjs")
    if hashlib.sha256(driver.read_bytes()).hexdigest() != DRIVER_HASH:
        raise ValueError("owned driver differs from the recorded experiment")
    sources = {}
    for name, expected in EXPECTED.items():
        with urllib.request.urlopen(BASE + name, timeout=30) as response:
            data = response.read()
        if hashlib.sha256(data).hexdigest() != expected:
            raise ValueError(f"published input changed: {name}")
        sources[name] = data.decode("utf-8")
    result = subprocess.run(
        ["node", str(driver)],
        input=json.dumps(sources).encode("utf-8"),
        stdout=subprocess.PIPE,
        check=True,
        timeout=180,
    )
    if hashlib.sha256(result.stdout).hexdigest() != OUTPUT_HASH:
        raise ValueError("oracle output differs from the recorded experiment")
    sys.stdout.buffer.write(result.stdout)


if __name__ == "__main__":
    main()
