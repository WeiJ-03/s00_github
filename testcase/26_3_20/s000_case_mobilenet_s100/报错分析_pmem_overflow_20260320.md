# PMEM Overflow 报错分析（2026-03-20-s000_case_mobilenet_s100）

> ./testcase/26_3_20/s000_case_mobilenet_s100

## 结论

当前 case 报错 `Error: pmem overflow!` 的根因是：该层输出特征图尺寸组合使 Psum 所需 PMEM 列深超过硬件建模上限 2048。

## 报错位置
- 触发检查代码：`s000_cmodel/source/module/nputop/kernels/Psum.cpp`
  - trim=1 分支检查 `col_max > 2048`
  - trim=0 分支检查 `col_max > 2048`
- PMEM 容量定义：`s000_cmodel/source/module/nputop/NPU.cpp`
  - `psum_mem_ = make_shared<Memory>(32, 8, 2048);`

## 本次 case 运行时关键参数
来自 `run_stdout_stderr.log` 报错前日志：
- `trim = 0`
- `ich_num = 128`
- `row_num = 38`
- `col_num = 112`

## 复算过程（对应 trim=0 分支）
源码公式：

- `row_frame = ((row_num - 1) / 8) + 1`
- `ch_group = (ich_num > 64) ? 4 : (ich_num / 16)`
- `col_max = row_frame * ch_group * col_num`

代入：

- `row_frame = ((38 - 1) / 8) + 1 = 5`
- `ch_group = 4`（因为 `ich_num = 128 > 64`）
- `col_max = 5 * 4 * 112 = 2240`

比较：
- `2240 > 2048`，因此触发 `Error: pmem overflow!`

## 补充现象
同一日志在报错前存在大量 `RBM Read Overflow` 打印，说明该层访存规模已逼近/超过当前建模容量，Psum 阶段被容量保护检查终止。
