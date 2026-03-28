#!/usr/bin/env python3
from __future__ import annotations

import argparse
import struct
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Print opflow.bin as 32-bit words for manual inspection"
    )
    parser.add_argument("opflow", type=Path, help="Path to opflow.bin")
    parser.add_argument(
        "--little-endian",
        action="store_true",
        help="Interpret words as little-endian (default: big-endian)",
    )
    parser.add_argument(
        "--start-word",
        type=int,
        default=0,
        help="Start printing from this word index (default: 0)",
    )
    parser.add_argument(
        "--count",
        type=int,
        default=-1,
        help="How many words to print (-1 means all)",
    )
    return parser.parse_args()


def load_words(path: Path, little_endian: bool) -> list[int]:
    raw = path.read_bytes()
    if len(raw) % 4 != 0:
        raise ValueError(f"File size is not multiple of 4 bytes: {len(raw)}")

    endian = "<" if little_endian else ">"
    word_count = len(raw) // 4
    return list(struct.unpack(f"{endian}{word_count}I", raw))


def main() -> int:
    args = parse_args()
    words = load_words(args.opflow, little_endian=args.little_endian)

    start = max(args.start_word, 0)
    if start >= len(words):
        print(f"total_words={len(words)}")
        print(f"start_word={start} is out of range")
        return 1

    if args.count < 0:
        end = len(words)
    else:
        end = min(start + args.count, len(words))

    print(f"file={args.opflow}")
    print(f"endianness={'little' if args.little_endian else 'big'}")
    print(f"total_bytes={len(words) * 4}")
    print(f"total_words={len(words)}")
    print(f"print_range=[{start}, {end - 1}]")
    print("idx      byte_off   hex         dec")

    for i in range(start, end):
        value = words[i]
        byte_off = i * 4
        print(f"{i:8d} {byte_off:10d} 0x{value:08x} {value:10d}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
