# s100_case_mobilenet 报错分析与编译调试建议_0322

## 现象

![image-20260322104520902](https://gitee.com/jiang-wei-2003/typora_fig/raw/master/image-20260322104520902.png)

- 运行到第一个 NPU 节点时打印 `Mac Call Count: 1`。
- 随后退出并打印 `Fatal exception: unknown`。

## 根因结论
- 实际异常是 Tensor 越界写入。
- 抛出位置：`s000_cmodel/source/utils/src/Tensor.cpp:213`
- 触发位置：`s000_cmodel/source/module/nputop/kernels/Mac.cpp:376`
- 触发函数：`Mac::ParseWeight()` 的 `CONV_MODE_1x1` 分支

## 关键证据（gdb）
- `Tensor<int>::Set(...)` 入参：`psum=32`
- 当前 `wt` 维度：`m_psumDim=32`
- 合法范围：`psum in [0, 31]`
- 判定：`psum` 越界，触发 `Error: ArrayIndexOutofBound inside Tensor`

## 为什么显示 unknown
- 主函数先捕获 `std::exception`，再 `catch (...)`。
- 本次抛出是 C 字符串异常（非 `std::exception`），因此打印为 `Fatal exception: unknown`。

## 直接触发原因
- `ParseWeight` 的 1x1 权重解析固定使用 `ich_group_32 < 4`（按 128 通道展开）。
- 当前 case 的实际输入通道不满足该固定展开假设。
- 结果是计算出的 `psum` 索引超出 `wt` 第二维长度。

## 详细拆解（为什么一定会越界）

![image-20260322110303731](https://gitee.com/jiang-wei-2003/typora_fig/raw/master/image-20260322110303731.png)

### Step 1：`wt` 的容量按实际输入通道创建
- 在 `Mac::ParseWeight()` 中先执行：`wt.Resize(och_num_, ich_num_, kernel_height_, kernel_width_)`。
- 本 case 的 gdb 结果显示：`m_psumDim=32`。
- 因此 `wt` 第二维合法下标只能是 `0~31`。

### Step 2：1x1 解权重循环固定按 4 组展开
- 1x1 分支里使用 `ich_group_32 < 4`。
- 每组再遍历 `ich_count=0~31`。
- 这等价于固定按 `4 x 32 = 128` 通道去展开写入。

### Step 3：写入下标公式
- 写入 `wt.Set(...)` 的第二维下标是：

```text
psum_index = ich_group * 128 + ich_group_32 * 32 + ich_count
```

- 该 case 中 `ich_num_=32`，因此 `ich_group_num_=1`，`ich_group` 只会是 `0`。
- 公式退化为：

```text
psum_index = ich_group_32 * 32 + ich_count
```

### Step 4：第一次越界发生点
- 当 `ich_group_32=0` 时，`psum_index=0~31`，合法。
- 当 `ich_group_32=1 且 ich_count=0` 时，`psum_index=32`，立刻越界。

对应表：

| ich_group_32 | ich_count 范围 | psum_index 范围 | 是否越界 |
|---|---:|---:|---|
| 0 | 0~31 | 0~31 | 否 |
| 1 | 0~31 | 32~63 | 是 |
| 2 | 0~31 | 64~95 | 是 |
| 3 | 0~31 | 96~127 | 是 |

### Step 5：与 gdb 现场完全一致
- gdb 抓到抛异常时参数是 `psum=32`。
- 同时 `m_psumDim=32`，允许最大下标 `31`。
- 因此异常必然触发：`Error: ArrayIndexOutofBound inside Tensor`。

`wt` 容量按 32 开，循环却按 128 写，越界是确定性结果。
