#!/usr/bin/env python3
import argparse
import os
import subprocess
from pathlib import Path


def build_command(case_case_path: Path, case_golden_path: Path, version: str):
    cmd = [
        './build/s000_model',
        '-c', str(case_case_path / 'config.bin'),
        '-i', str(case_case_path / 'cmd.bin'),
        '-w', str(case_case_path / 'weight.bin'),
        '-f', str(case_case_path / 'opflow.bin'),
        '-j', str(case_case_path / 'map.json'),
        '-l', str(case_case_path / 'lut.bin'),
        '-g', str(case_golden_path) + '/',
    ]
    if version == 'v006':
        cmd.extend(['-d', str(case_golden_path / 'input_1.bin'), '-d', str(case_golden_path / 'input_2.bin')])
    else:
        cmd.extend(['-d', str(case_golden_path / 'input.bin')])
    return cmd


def run_single_case(case_path: Path, version: str):
    case_case_path = case_path / 'case'
    case_golden_path = case_path / 'golden'

    result_file = Path('result_single_case.txt')
    if result_file.exists():
        result_file.unlink()

    cmd = build_command(case_case_path, case_golden_path, version)
    completed = subprocess.run(cmd, cwd='.', text=True)
    ret = completed.returncode >> 8

    result_flag = '0'
    if result_file.exists():
        result_flag = (result_file.read_text().strip() or '0')

    is_pass = result_flag.startswith('1')
    status = 'Right' if is_pass else 'Wrong'
    return status, ret, result_flag


def run_all_cases(testcase_dir: Path, version: str, convmode: str):
    case_dirs = sorted([p for p in testcase_dir.iterdir() if p.is_dir()])
    total = len(case_dirs)
    print(f'Total test cases: {total}')

    with open('./result.txt', 'w') as out:
        for idx, case_path in enumerate(case_dirs, 1):
            print(f'[{idx:3d}/{total}] Run {(version+convmode)} {case_path.name} ...', end=' ')
            status, ret, result_flag = run_single_case(case_path, version)
            out.write(f'{case_path.name}       {status}\n')
            print(f'{status} (ret={ret}, result_single_case={result_flag})')


def main():
    parser = argparse.ArgumentParser(description='Simple cmodel regression')
    parser.add_argument('--conv_mode', default='', type=str, help='Conv mode of test case')
    parser.add_argument('--version', default='', type=str, help='Case version')
    parser.add_argument('--testcase_dir', default='./testcase/26_1_30/s000_testcase/v001_BYPASS', help='Root dir containing cases')
    args = parser.parse_args()

    testcase_dir = Path(args.testcase_dir)
    if not testcase_dir.exists():
        print(f'Error: Test case directory not found: {testcase_dir}')
        return 1

    run_all_cases(testcase_dir, args.version, args.conv_mode)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
