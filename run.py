import argparse
import os
import subprocess
import numpy as np
import time

def GetPath(tmp, casePath):
    path_list = os.listdir(tmp)
    for pl in path_list:
        if casePath in pl:
            return pl


def Regression(convmode: str, version: str):
    root = os.path.join('./testcase', (version+'_'+convmode),(version+'_'+convmode))
    testcase = os.listdir(root)

    for case in testcase:
        casepath = os.path.join(root,case,'case')
        goldenpath = os.path.join(root,case,'golden')
        cmodel = './build/s000_model'
        cmodel += " -c " + casepath + '/config.bin'
        cmodel += " -i " + casepath + '/cmd.bin'
        cmodel += " -w " + casepath + '/weight.bin'
        cmodel += " -f " + casepath + '/opflow.bin'
        cmodel += " -j " + casepath + '/map.json'
        cmodel += " -l " + casepath + '/lut.bin'
        cmodel += " -g " + goldenpath + '/'        

        if (version=='v006'):
            cmodel += " -d " + goldenpath + '/input_1.bin'
            cmodel += " -d " + goldenpath + '/input_2.bin'
        else:
            cmodel += " -d " + goldenpath + '/input.bin'
        print("Run ",(version+convmode))
        os.system(cmodel)
        ret = -10
        ret = os.system(cmodel)
        ret = ret >> 8
        while (ret==-10):
            pass

        f = open("./result.txt","a")
        f_result = open("./result_single_case.txt")
        r_or_w = f_result.read()
        if ret != 0:    
            r_or_w = '0'
        print(r_or_w)
        

        if r_or_w == '1':
            f.write(case + "       " + "Right" + "\n")
        else:
            f.write(case + "       " + "Wrong" + "\n")
        f.close()
        f_result.close()



def Run(argv):
    Regression(argv.conv_mode, argv.version)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Cmodel Regression")
    parser.add_argument('--conv_mode',default='BYPASS',type=str,help='Conv mode of test case')
    parser.add_argument('--version',default='v001',type=str,help='case version')
    argv = parser.parse_args()
    Run(argv)
