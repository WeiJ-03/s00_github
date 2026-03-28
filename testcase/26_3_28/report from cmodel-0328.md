> s000_case_resnet_cut_1layer  PASS
>
> s000_case_resnet_cut_0310 PASS
>
> s000_case_resnet50 Fali
>
> s000_case_mobilenet Fail
>
> mobilenet-1layer: no golden from iee , can't test





## s000_case_resnet50

到第87个node出现error：

ERROR: 

**Cannot open file：**

**./testcase/26_3_28/s000_case_resnet50/golden/Bypass__layer1_layer1.1_conv3_Conv_and_hardwarefusion_7.bin**

![image-20260328130432093](https://gitee.com/jiang-wei-2003/typora_fig/raw/master/image-20260328130432093.png)

## s000_case_mobilenet

**RBM Read Overflow in Node 51**

![image-20260328131446463](https://gitee.com/jiang-wei-2003/typora_fig/raw/master/image-20260328131446463.png)

![image-20260328131527729](https://gitee.com/jiang-wei-2003/typora_fig/raw/master/image-20260328131527729.png)