from pathlib import Path
import struct
from typing import Tuple

# Cases to patch
CASES = [
    14, 15, 16, 17, 18,
    19, 20, 21, 22, 40,
    43, 49, 50, 51, 52,
    53, 64, 65, 66, 77,
    89, 90
]

# Old/new LUT base addresses
OLD = 0x80600000
NEW = 0x40600000

ROOT = Path(__file__).resolve().parent.parent


def patch_file(p: Path) -> Tuple[int, int]:
    data = p.read_bytes()
    words = [int.from_bytes(data[i:i + 4], "big") for i in range(0, len(data), 4)]
    idxs = [i for i, w in enumerate(words) if w == OLD]
    if not idxs:
        return 0, 0
    for idx in idxs:
        words[idx] = NEW
    out = b"".join(struct.pack(">I", w) for w in words)
    # backup once
    bak = p.with_suffix(p.suffix + ".bak")
    if not bak.exists():
        bak.write_bytes(data)
    p.write_bytes(out)
    return len(idxs), len(words)


def main() -> None:
    patched = 0
    missing = 0
    unchanged = 0
    for num in CASES:
        case_dir = ROOT / "testcase/26_1_30/s000_testcase/v001_BYPASS" / f"{num:05d}_v001_BYPASS" / "case"
        p = case_dir / "opflow.bin"
        if not p.exists():
            missing += 1
            print(f"[MISS] {p}")
            continue
        hits, total = patch_file(p)
        if hits:
            patched += 1
            print(f"[PATCH] {p} hits={hits} words={total}")
        else:
            unchanged += 1
            print(f"[SKIP] {p} no OLD value found")
    print(f"done. patched={patched} unchanged={unchanged} missing={missing}")


if __name__ == "__main__":
    main()
