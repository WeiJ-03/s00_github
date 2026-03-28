# s000_case_vit_cut_1 opflow 报错分析

## 现象

使用下面这组参数运行模型时：

```bash
./build/s000_model \
  -c ./testcase/26_3_7/s000_case_vit_cut_1/config.bin \
  -i ./testcase/26_3_7/s000_case_vit_cut_1/cmd.bin \
  -d ./testcase/26_3_7/s000_case_vit_cut_1/golden/input.bin \
  -w ./testcase/26_3_7/s000_case_vit_cut_1/weight.bin \
  -f ./testcase/26_3_7/s000_case_vit_cut_1/opflow.bin \
  -j ./testcase/26_3_7/s000_case_vit_cut_1/map.json \
  -g ./testcase/26_3_7/s000_case_vit_cut_1/golden/ \
  -l ./testcase/26_3_7/s000_case_vit_cut_1/lut.bin
```

程序输出：

```text
[Info] Opflow is specified
[Info] Lut is specified
[Info] Json is specified
Error: Unsupported node type!!
```

报错位置在 `Graph::Graph()` / `Graph::ParseCPUNode()` 的解析阶段，不是在 NPU 执行阶段。

## 对比对象

正常 case：

- `testcase/26_1_30/s000_testcase/v001_BYPASS/00001_v001_BYPASS/case/opflow.bin`

失败 case：

- `testcase/26_3_7/s000_case_vit_cut_1/opflow.bin`

## 根因结论

第二个 case 的 `opflow.bin` 编码格式和当前 cmodel 解析器假设的不一致。

当前 cmodel 在 `s000_cmodel/source/graph/Graph.cpp` 中，默认按 **每个字段 4 字节** 顺序读取：

1. `node_index`
2. `node_type`
3. 如果不是 DataNode，再读取 `next_node_idx`
4. 再继续读取 CPU/NPU/DataNode 的具体字段

这个结论可以直接从 `GetOneWordValue()` 看出来：

- 函数内部固定循环 4 次
- 每次循环读取 1 个 byte，并执行一次 `idx++`
- 所以每调用一次 `GetOneWordValue()`，就会顺序消费 4 个 byte，也就是 1 个 32-bit 字段

```c++
int Graph::GetOneWordValue(vector<uint8_t> const& opflow, unsigned long& idx) {
    int value = 0;
    for (int j = 0; j < 4; j++) {       //32bit读取
        value += ((int) opflow[idx++]) << 8*(3-j);
    }
    return value;
}
```

`Graph::Graph()` 里又是连续调用 `GetOneWordValue()` 去读取 `node_index`、`node_type`、`next_node_idx`，所以整个 parser 都建立在“字段宽度固定为 4 字节”的假设上

第一个正常 case 的 `opflow.bin` 正是这种 32-bit 连续字段格式，所以能够正常解析。

第二个失败 case 的 `opflow.bin` 实际上更像是 **每个字段占 8 字节槽位，真实值落在后 4 字节** 的格式。也就是说，当前 parser 以 32-bit 连续读取时，会把大量高 32 位的 `0` 当成真正字段，导致后续所有字段错位。

错位之后，本来应该被当成 `start_addr` 的值，会被误读成 `cpu_node_type` 或 `node_type`，于是落到 parser 的 `default` 分支，最终打印：

```text
Error: Unsupported node type!!
```

## 可能的证明

### 1. 正常 case 的前几个 32-bit word 是连续有效的

正常 case 前几个 word：

```text
0, 0, 1, 0, 0x81000000, 0x4c,
1, 0, 2, 1, 0x90000000, 0x410,
2, 0, 3, 2, 100000, 0, ...
```

之所以说它们是“连续有效”的，是因为把这些值按当前 parser 的读取顺序代进去，正好可以连续拼成合法节点，没有发生字段错位：

- `0, 0, 1, 0, 0x81000000, 0x4c`
  - `node_index = 0`
  - `node_type = 0`，表示 CPU node
  - `next_node_idx = 1`
  - `cpu_node_type = 0`，表示 CDMA
  - 后两个字段正好对应 `start_addr` 和 `len`
- `1, 0, 2, 1, 0x90000000, 0x410`
  - `node_index = 1`
  - `node_type = 0`，表示 CPU node
  - `next_node_idx = 2`
  - `cpu_node_type = 1`，表示 WDMA
  - 后两个字段正好对应 `start_addr` 和 `len`
- `2, 0, 3, 2, 100000, 0, ...`
  - `node_index = 2`
  - `node_type = 0`，表示 CPU node
  - `next_node_idx = 3`
  - `cpu_node_type = 2`，表示 DIDMA
  - 后面的多个字段继续按 DIDMA 结构顺序对齐

这正好能对应当前 parser 期待的结构：

- node 0: CPU CDMA
- node 1: CPU WDMA
- node 2: CPU DIDMA

### 2. 失败 case 的前几个 32-bit word 呈现“隔位有效”特征

失败 case 前几个 32-bit word：

```text
0, 0, 0, 0, 0, 1,
0, 0, 0, 0x40000000, 0, 0x0a04,
0, 1, 0, 0, 0, 2,
0, 1, 0, 0x50000000, 0, 0x01d5d400, ...
```

可以看到：

- 很多真实值前面都插着一个 `0`
- 真实值并不是每 4 字节连续排布
- 这和“每个字段 8 字节，高 4 字节经常为 0，低 4 字节才是有效值”的特征一致

### 4. 把失败 case 按 64-bit field 看，结构反而是合理的

失败 case 如果按 8 字节一组读取，前几个字段会变成：

```text
0, 0, 1, 0, 0x40000000, 0x0a04,
1, 0, 2, 1, 0x50000000, 0x01d5d400,
2, 0, 3, 2, 100000, 0, 7168, 64, ...
```

这说明文件本身不是随机损坏，而是采用了不同的字段打包方式。

### 5. 当前 parser 读到第二个节点时就已经错位

按现在的 32-bit parser 去模拟读取失败 case，大致会读成：

```text
step 0: node_index=0, node_type=0, next=0, cpu_node_type=0
step 1: node_index=0, node_type=0, next=0, cpu_node_type=1073741824
```

这里的 `1073741824` 实际上是 `0x40000000`，它本应是地址字段，但因为整体错位，被错误当成了 `cpu_node_type`。

当前代码只支持 `0..10` 这些 CPU node type，因此立即报错。

## 为什么第一个 case 不报错

因为第一个 case 的 `opflow.bin` 和 `Graph.cpp` 的解析逻辑是匹配的：

- 文件按 32-bit 字段连续编码
- parser 也按 32-bit 字段连续消费
- 所以节点边界、node type、地址字段都能对齐

## 为什么第二个 case 会报错

因为第二个 case 的 `opflow.bin` 与当前 parser 的字段宽度假设不匹配：

- 文件更像 64-bit field 对齐
- parser 仍按 32-bit field 解析
- 从第二个节点开始字段就错位
- 地址等普通字段被误认成 `node_type` 或 `cpu_node_type`
- 最终触发 `Unsupported node type!!`

## 后续修复方向

可选方案有两个：

### 方案 1-难度较大

修改 `s000_cmodel/source/graph/Graph.cpp`，让 parser 同时兼容：

- 传统 32-bit opflow
- 64-bit field 对齐的 opflow

>
>
>我这边尝试了解析方式改为64bit，可以通过opflow.bin的解析，但是cmd.bin也会报错
>
>**麻烦重点检查编译器那边生成的位宽问题**，Cmodel这边应该是默认32bit（4Byte）解析，但是现在给过来的都是64bit，初步判断是位宽问题

### 方案 2-修改opflow位宽即可，相对容易

从 testcase 生成端修正导出逻辑，把重新导出成与第一个 case 一致的 32-bit 连续字段格式。

