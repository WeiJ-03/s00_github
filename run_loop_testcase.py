#!/usr/bin/env python3
import os
import subprocess
import sys
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


def read_result_flag() -> str:
    result_file = Path('result_single_case.txt')
    if result_file.exists():
        return result_file.read_text().strip() or '0'
    return '0'

def run_all_testcases(testcase_dir, dumpfile_dir, version: str = ''):
    """
    循环运行testcase目录下的所有case
    每个case的dumpfile会保存到 dumpfile/testcase/<case_name>/
    """
    # 获取所有case目录
    case_dirs = sorted([d for d in os.listdir(testcase_dir) 
                        if os.path.isdir(os.path.join(testcase_dir, d))])
    
    total_cases = len(case_dirs)
    print(f"Total test cases found: {total_cases}")
    print(f"Test case directory: {testcase_dir}")
    print("-" * 80)
    
    passed_cases = []
    failed_cases = []
    error_cases = []
    
    for idx, case_name in enumerate(case_dirs, 1):
        case_path = Path(testcase_dir) / case_name
        case_case_path = case_path / 'case'
        case_golden_path = case_path / 'golden'
        
        
        # 创建对应的dumpfile输出目录
        case_dumpfile_dir = os.path.join(dumpfile_dir, case_name)
        os.makedirs(case_dumpfile_dir, exist_ok=True)
        
        print(f"[{idx:3d}/{total_cases}] Running: {case_name}...", end=" ", flush=True)
        
        # 构建命令
        result_file = Path('result_single_case.txt')
        if result_file.exists():
            result_file.unlink()

        cmd = build_command(case_case_path, case_golden_path, version)
        
        # 设置环境变量，将dumpfile输出到指定目录
        env = os.environ.copy()
        env['DUMPFILE_DIR'] = case_dumpfile_dir
        
        # 运行程序
        try:
            result = subprocess.run(cmd, cwd='.', env=env, capture_output=True, text=True, timeout=300)
            result_flag = read_result_flag()
            is_pass = result_flag.startswith('1')
            if is_pass:
                print("✓ PASSED")
                passed_cases.append(case_name)
            else:
                print(f"✗ FAILED (exit code: {result.returncode}, result_single_case={result_flag})")
                failed_cases.append(case_name)
                if result.stdout:
                    print(f"    stdout: {result.stdout[:100]}")
                if result.stderr:
                    print(f"    stderr: {result.stderr[:100]}")
        except subprocess.TimeoutExpired:
            print("✗ TIMEOUT")
            error_cases.append((case_name, "Timeout"))
        except Exception as e:
            print(f"✗ ERROR: {str(e)[:50]}")
            error_cases.append((case_name, str(e)))
    
    # 打印总结
    print("-" * 80)
    print(f"\n测试总结:")
    print(f"  总用例数:     {total_cases}")
    print(f"  通过:         {len(passed_cases)} ({100*len(passed_cases)//total_cases}%)")
    print(f"  失败:         {len(failed_cases)}")
    print(f"  错误:         {len(error_cases)}")
    
    if failed_cases:
        print(f"\n失败的用例:")
        for case in failed_cases[:10]:  # 只显示前10个
            print(f"  - {case}")
        if len(failed_cases) > 10:
            print(f"  ... 还有 {len(failed_cases) - 10} 个")
    
    if error_cases:
        print(f"\n出现错误的用例:")
        for case, error in error_cases[:5]:  # 只显示前5个
            print(f"  - {case}: {error}")
        if len(error_cases) > 5:
            print(f"  ... 还有 {len(error_cases) - 5} 个")
    
    print(f"\n所有dumpfile已保存到: {case_dumpfile_dir}")
    print("=" * 80 + "\n")
    
    return len(failed_cases) == 0 and len(error_cases) == 0

if __name__ == "__main__":
    testcase_dir = './testcase/26_1_30/s000_testcase/v006_ELE_ADD'
    dumpfile_dir = './dumpfile/testcase/v006_ELE_ADD'
    version = 'v006' if 'v006' in testcase_dir else ''
    
    if not os.path.exists(testcase_dir):
        print(f"Error: Test case directory not found: {testcase_dir}")
        sys.exit(1)
    
    success = run_all_testcases(testcase_dir, dumpfile_dir, version)
    sys.exit(0 if success else 1)

