#!/usr/bin/env python3
from __future__ import annotations

import argparse
import re
from pathlib import Path
from typing import Iterable, List


LINE_RE = re.compile(r"^(\d{5})_.*\bWrong\b", re.IGNORECASE)


def iter_wrong_cases(lines: Iterable[str]) -> List[int]:
    cases: List[int] = []
    for line in lines:
        m = LINE_RE.search(line.strip())
        if not m:
            continue
        cases.append(int(m.group(1)))
    return cases


def format_cases(cases: List[int], per_line: int = 5) -> str:
    if not cases:
        return "CASES = []"
    chunks = [cases[i : i + per_line] for i in range(0, len(cases), per_line)]
    lines = ["CASES = ["]
    for chunk in chunks:
        line = "    " + ", ".join(str(n) for n in chunk)
        if chunk != chunks[-1]:
            line += ","
        lines.append(line)
    lines.append("]")
    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(description="Extract wrong case indices from result file.")
    parser.add_argument("result_file", type=Path, help="result-*.txt file")
    parser.add_argument("--per-line", type=int, default=5, help="numbers per line")
    args = parser.parse_args()

    text = args.result_file.read_text(encoding="utf-8", errors="ignore")
    cases = iter_wrong_cases(text.splitlines())
    print(format_cases(cases, per_line=args.per_line))


if __name__ == "__main__":
    main()
