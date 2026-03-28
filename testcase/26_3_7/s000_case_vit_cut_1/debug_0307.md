python3 scripts/print_opflow_words.py testcase/26_3_7/s000_case_vit_cut_1/opflow.bin --count 40
python3 scripts/print_opflow_words.py testcase/26_3_7/s000_case_vit_cut_1/opflow.bin --start-word 6 --count 20

----

![image-20260307200518374](https://gitee.com/jiang-wei-2003/typora_fig/raw/master/20260307200519.png)

报错：Error: Unsupported node type!!



我打印了opflow.bin文件

![image-20260307200432465](https://gitee.com/jiang-wei-2003/typora_fig/raw/master/20260307200434.png)

很明显溢出了不匹配，
![image-20260307200657034](https://gitee.com/jiang-wei-2003/typora_fig/raw/master/20260307200657.png)