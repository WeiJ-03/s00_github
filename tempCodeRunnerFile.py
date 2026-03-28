
    print("-" * 80)
    
    passed_cases = []
    failed_cases = []
    error_cases = []
    
    for idx, case_name in enumerate(case_dirs, 1):
        case_path = os.path.join(testcase_dir, case_name)
        case_case_path = os.path.join(case_path, 'case')
        case_golden_path = os.path.join(case_path, 'golden')
        
        # 创建对应的dumpfile输出目录
        dumpfile_dir = os.path.join('./dumpfile/testcase', case_name)
        os.makedirs(dumpfile_dir, exist_ok=True)
        
        print(f"[{idx:3d}/{total_cases}] Running: {case_name}...", end=" ", flush=True)
        
        # 构建命令
        cmd = [
            './build/s000_model',