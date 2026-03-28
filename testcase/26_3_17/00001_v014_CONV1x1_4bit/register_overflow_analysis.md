# 寄存器溢出（案例：26_3_17/00001_v014_CONV1x1_4bit）

> 1. 核心故障是寄存器索引越界：代码仅支持16个寄存器（索引0-15），但案例指令尝试写入索引16-18；
> 2. 索引越界的根源是`cmd.bin`解码后的指令包含非法寄存器索引，且解码逻辑会将原始索引减2进一步放大越界问题；
> 3. 异常触发点为`NPU.cpp`的`NPU::Run()`函数中`register_value_.at()`的范围检查。
>
> 
> 可能的解决方案：编译器请检查cmd.bin的REGWRITE命令相关的idx是否存在越界



## 故障现象

运行时错误：

![image-20260317213904369](https://gitee.com/jiang-wei-2003/typora_fig/raw/master/image-20260317213904369.png)

- `Fatal exception: vector::_M_range_check: __n (which is 16) >= this->size() (which is 16)`

## 直接崩溃位置

- 源码文件：`s000_cmodel/source/module/nputop/NPU.cpp`
- 函数：`NPU::Run()`
- 崩溃语句：`register_value_.at(reg_target_idx_) = reg_target_value_;`
- `register_value_` 的大小为16（初始化为`vector<int>(16, 0)`），有效索引范围是`[0, 15]`。

## 哪个解码文件导致寄存器越界

溢出问题源于**该案例文件解码后的命令流**：
- `testcase/26_3_17/00001_v014_CONV1x1_4bit/case/cmd.bin`

代码中的关键解码流程：
1. `main.cpp` 通过`LoadBinVector_for_cmd(...)`函数加载命令（cmd）
2. `Utility/IO/FileIO.cpp` 中的`LoadBinVector_for_cmd`函数会对每个4字节的字进行字节反转
3. `CommandList::ParseREGWRITECMD` 函数解码出`target_index`（目标索引）
4. `NPU::Run` 函数通过`reg_target_idx_ = target_index - 2`将其转换为向量索引

因此，这并非直接解析原始字节数据导致的问题，而是对每个字完成字节反转后，运行时解码得到的命令字引发的异常。

## 解码出的违规命令

使用与运行时相同的逻辑（`LoadBinVector_for_cmd`函数）解码`cmd.bin`后，发现存在以下寄存器写入（REGWRITE）命令：
- `target_index = 18` → `reg_target_idx = 16`（第一个无效索引）
- `target_index = 19` → `reg_target_idx = 17`
- `target_index = 20` → `reg_target_idx = 18`

由于寄存器影子向量仅有16个元素，`reg_target_idx = 16`时会立即触发范围检查异常。

## 根本原因总结

- 当前的模型实现仅支持/镜像映射16个寄存器槽位；
- 该v014 4bit版本的案例在`cmd.bin`中尝试写入更高的寄存器索引（18/19/20）；
- 因此在`NPU::Run`函数中，`vector::at`的范围检查失败，触发运行时异常。

## 结论

寄存器越界问题由**以下文件解码后的指令**导致：
- `testcase/26_3_17/00001_v014_CONV1x1_4bit/case/cmd.bin`

且异常具体出现在：
- `s000_cmodel/source/module/nputop/NPU.cpp` 文件的`NPU::Run`函数中，执行`register_value_.at(reg_target_idx_)`写入操作时。

### 

