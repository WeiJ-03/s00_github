#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, List, Optional


SUPPORTED_CPU_TYPES = {
    0: "CDMA",
    1: "WDMA",
    2: "DIDMA",
    3: "DODMA",
    4: "CONCAT",
    5: "PDMA",
    6: "CPUSHIFT",
    7: "LUTDMA",
    8: "L2N",
    9: "RESHAPE",
    10: "SOFTMAX",
}

NODE_TYPE_NAMES = {
    0: "CPU_NODE",
    1: "NPU_NODE",
    2: "DATA_NODE",
}


class OpflowParseError(Exception):
    def __init__(self, message: str, *, word_index: int, node_index: Optional[int]) -> None:
        super().__init__(message)
        self.word_index = word_index
        self.node_index = node_index


@dataclass
class Cursor:
    words: List[int]
    i: int = 0

    @property
    def total(self) -> int:
        return len(self.words)

    def need(self, count: int, *, node_index: Optional[int]) -> None:
        if self.i + count > self.total:
            raise OpflowParseError(
                f"Unexpected EOF: need {count} words, remaining {self.total - self.i}",
                word_index=self.i,
                node_index=node_index,
            )

    def read_u32(self, *, node_index: Optional[int]) -> int:
        self.need(1, node_index=node_index)
        value = self.words[self.i]
        self.i += 1
        return value


class OpflowParser:
    def __init__(self, words: List[int]) -> None:
        self.cur = Cursor(words)

    def parse(self, max_nodes: Optional[int] = None) -> List[Dict[str, Any]]:
        nodes: List[Dict[str, Any]] = []
        while self.cur.i < self.cur.total:
            if max_nodes is not None and len(nodes) >= max_nodes:
                break
            nodes.append(self._parse_one_node())
        return nodes

    def _parse_one_node(self) -> Dict[str, Any]:
        node_start = self.cur.i
        node_index = self.cur.read_u32(node_index=None)
        node_type = self.cur.read_u32(node_index=node_index)

        if node_type not in NODE_TYPE_NAMES:
            raise OpflowParseError(
                f"Unsupported node_type={node_type}",
                word_index=self.cur.i - 1,
                node_index=node_index,
            )

        next_node_idx: Optional[int] = None
        if node_type != 2:
            next_node_idx = self.cur.read_u32(node_index=node_index)

        node: Dict[str, Any] = {
            "node_index": node_index,
            "node_type": node_type,
            "node_type_name": NODE_TYPE_NAMES[node_type],
            "next_node_idx": next_node_idx,
            "word_start": node_start,
        }

        if node_type == 0:
            node.update(self._parse_cpu_body(node_index))
        elif node_type == 1:
            node["body"] = {}
        else:
            node.update(self._parse_data_body(node_index))

        node["word_end"] = self.cur.i - 1
        node["word_len"] = self.cur.i - node_start
        return node

    def _parse_cpu_body(self, node_index: int) -> Dict[str, Any]:
        cpu_type_word = self.cur.i
        cpu_type = self.cur.read_u32(node_index=node_index)
        if cpu_type not in SUPPORTED_CPU_TYPES:
            raise OpflowParseError(
                f"Unsupported cpu_node_type={cpu_type} (0x{cpu_type:08x})",
                word_index=cpu_type_word,
                node_index=node_index,
            )

        name = SUPPORTED_CPU_TYPES[cpu_type]
        body: Dict[str, Any] = {"cpu_node_type": cpu_type, "cpu_node_type_name": name}

        if cpu_type in (0, 1, 5):
            body["start_addr"] = self.cur.read_u32(node_index=node_index)
            body["len"] = self.cur.read_u32(node_index=node_index)
        elif cpu_type == 2:
            body["input_node_idx"] = self.cur.read_u32(node_index=node_index)
            body["offset"] = self.cur.read_u32(node_index=node_index)
            body["row_pitch_len"] = self.cur.read_u32(node_index=node_index)
            body["row_pitch_num"] = self.cur.read_u32(node_index=node_index)
            body["row_len"] = self.cur.read_u32(node_index=node_index)
            body["ch_pitch_len"] = self.cur.read_u32(node_index=node_index)
            body["ch_pitch_num"] = self.cur.read_u32(node_index=node_index)
            sram_offset = self.cur.read_u32(node_index=node_index)
            body["sram_offset_raw"] = sram_offset
            body["sram_offset_x"] = sram_offset // 16
            body["sram_offset_y"] = sram_offset % 16
            body["dmem"] = self.cur.read_u32(node_index=node_index)
            body["doAdd"] = self.cur.read_u32(node_index=node_index)
        elif cpu_type == 3:
            body["output_node_idx"] = self.cur.read_u32(node_index=node_index)
            body["offset"] = self.cur.read_u32(node_index=node_index)
            body["row_pitch_len"] = self.cur.read_u32(node_index=node_index)
            body["row_pitch_num"] = self.cur.read_u32(node_index=node_index)
            body["row_len"] = self.cur.read_u32(node_index=node_index)
            body["ch_pitch_len"] = self.cur.read_u32(node_index=node_index)
            body["ch_pitch_num"] = self.cur.read_u32(node_index=node_index)
            sram_offset = self.cur.read_u32(node_index=node_index)
            body["sram_offset_raw"] = sram_offset
            body["sram_offset_x"] = sram_offset // 16
            body["sram_offset_y"] = sram_offset % 16
            body["smem"] = self.cur.read_u32(node_index=node_index)
        elif cpu_type == 4:
            body["axis"] = self.cur.read_u32(node_index=node_index)
            num_inputs = self.cur.read_u32(node_index=node_index)
            body["num_of_input_nodes"] = num_inputs
            body["input_nodes_idx"] = [self.cur.read_u32(node_index=node_index) for _ in range(num_inputs)]
            body["output_node_idx"] = self.cur.read_u32(node_index=node_index)
        elif cpu_type == 6:
            body["input_node_idx"] = self.cur.read_u32(node_index=node_index)
            body["output_node_idx"] = self.cur.read_u32(node_index=node_index)
            body["shift_en"] = self.cur.read_u32(node_index=node_index)
            body["start_addr"] = self.cur.read_u32(node_index=node_index)
            body["len"] = self.cur.read_u32(node_index=node_index)
        elif cpu_type == 7:
            body["lut_start_addr"] = self.cur.read_u32(node_index=node_index)
        elif cpu_type == 8:
            body["input_node_idx"] = self.cur.read_u32(node_index=node_index)
            body["output_node_idx"] = self.cur.read_u32(node_index=node_index)
            body["start_addr"] = self.cur.read_u32(node_index=node_index)
        elif cpu_type in (9, 10):
            body["input_node_idx"] = self.cur.read_u32(node_index=node_index)
            body["output_node_idx"] = self.cur.read_u32(node_index=node_index)

        return {"body": body}

    def _parse_data_body(self, node_index: int) -> Dict[str, Any]:
        body: Dict[str, Any] = {
            "addr": self.cur.read_u32(node_index=node_index),
            "format": self.cur.read_u32(node_index=node_index),
            "height": self.cur.read_u32(node_index=node_index),
            "width": self.cur.read_u32(node_index=node_index),
            "channel": self.cur.read_u32(node_index=node_index),
        }

        if body["format"] == 3:
            chunk_num = self.cur.read_u32(node_index=node_index)
            body["chunk_number"] = chunk_num
            body["chunk_start"] = [self.cur.read_u32(node_index=node_index) for _ in range(chunk_num)]

        return {"body": body}


def read_words_be(path: Path) -> List[int]:
    data = path.read_bytes()
    if len(data) % 4 != 0:
        raise ValueError(f"File size is not 4-byte aligned: {len(data)} bytes")
    return list(struct.unpack(f">{len(data)//4}I", data))


def build_summary(nodes: List[Dict[str, Any]], total_words: int) -> Dict[str, Any]:
    by_node_type: Dict[str, int] = {}
    by_cpu_type: Dict[str, int] = {}

    for n in nodes:
        nt = n["node_type_name"]
        by_node_type[nt] = by_node_type.get(nt, 0) + 1
        body = n.get("body", {})
        if nt == "CPU_NODE":
            name = body.get("cpu_node_type_name", "UNKNOWN")
            by_cpu_type[name] = by_cpu_type.get(name, 0) + 1

    return {
        "node_count": len(nodes),
        "total_words": total_words,
        "by_node_type": by_node_type,
        "by_cpu_type": by_cpu_type,
    }


def print_human(nodes: List[Dict[str, Any]], summary: Dict[str, Any], max_print: int) -> None:
    print("=== Opflow Summary ===")
    print(f"nodes={summary['node_count']} words={summary['total_words']}")
    print(f"node_type_count={summary['by_node_type']}")
    if summary["by_cpu_type"]:
        print(f"cpu_type_count={summary['by_cpu_type']}")
    print()

    show = nodes if max_print < 0 else nodes[:max_print]
    for n in show:
        print(
            f"node={n['node_index']} type={n['node_type_name']} "
            f"next={n['next_node_idx']} words=[{n['word_start']},{n['word_end']}]"
        )
        body = n.get("body", {})
        if n["node_type_name"] == "CPU_NODE":
            print(
                f"  cpu_type={body.get('cpu_node_type_name')}({body.get('cpu_node_type')})"
            )
        print(f"  body={body}")

    if max_print >= 0 and len(nodes) > max_print:
        print(f"... ({len(nodes) - max_print} more nodes omitted)")


def make_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description="Parse opflow.bin according to current Graph.cpp format")
    p.add_argument("opflow", type=Path, help="Path to opflow.bin")
    p.add_argument("--json", dest="as_json", action="store_true", help="Output full parse result as JSON")
    p.add_argument(
        "--max-print",
        type=int,
        default=20,
        help="Max nodes to print in text mode (-1 to print all)",
    )
    p.add_argument(
        "--max-parse",
        type=int,
        default=None,
        help="Stop parsing after this many nodes (for quick inspection)",
    )
    return p


def main() -> int:
    args = make_parser().parse_args()

    try:
        words = read_words_be(args.opflow)
        parser = OpflowParser(words)
        nodes = parser.parse(max_nodes=args.max_parse)
        summary = build_summary(nodes, total_words=len(words))

        if args.as_json:
            result = {
                "file": str(args.opflow),
                "summary": summary,
                "nodes": nodes,
            }
            print(json.dumps(result, indent=2, ensure_ascii=True))
        else:
            print_human(nodes, summary, max_print=args.max_print)
        return 0
    except OpflowParseError as e:
        error_payload = {
            "error": str(e),
            "word_index": e.word_index,
            "byte_offset": e.word_index * 4,
            "node_index": e.node_index,
        }
        print(json.dumps(error_payload, indent=2, ensure_ascii=True))
        return 2
    except Exception as e:  # keep CLI robust for malformed files
        print(json.dumps({"error": str(e)}, indent=2, ensure_ascii=True))
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
