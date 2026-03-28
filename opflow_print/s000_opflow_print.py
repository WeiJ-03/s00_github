import os
import struct
import sys

def print_info(node_type, node_name, node_value):
    print(node_type.rjust(7), ':', node_name.ljust(20), '=', hex(node_value).ljust(15), node_value)

def GetValue(f_in, node_type, node_name):
    node_value = struct.unpack('>L', f_in.read(4))[0]
    print_info(node_type, node_name, node_value)

file_in = 'opflow.bin'

CDMA_node   = ['node_index', 'node_type', 'next_node_index', 'opcode_id', 'addr', 'len']
WDMA_node   = ['node_index', 'node_type', 'next_node_index', 'opcode_id', 'addr', 'len']
DIDMA_node  = ['node_index', 'node_type', 'next_node_index', 'opcode_id', 'input_Node_index', 'dram_offset', 'rowPitchLen', \
               'rowPitchNum', 'rowLen', 'chPitchLen', 'chPitchNum', 'sram_offset', 'dmem', 'preprocessType']
DODMA_node  = ['node_index', 'node_type', 'next_node_index', 'opcode_id', 'output_Node_index', 'dram_offset', 'rowPitchLen', \
               'rowPitchNum', 'rowLen', 'chPitchLen', 'chPitchNum', 'sram_offset', 'dmem']
PDMA_node   = ['node_index', 'node_type', 'next_node_index', 'opcode_id', 'bitwidth', 'addr', 'len']
LUTDMA_node = ['node_index', 'node_type', 'next_node_index', 'opcode_id', 'addr']

data_node   = ['node_index', 'node_type', 'dram_addr', 'format', 'height', 'width', 'channel']

f_in = open(file_in, 'rb')
file_size = os.path.getsize(file_in)
count = 0
while (count < file_size):
    node_index = struct.unpack('>L', f_in.read(4))[0]
    node_type = struct.unpack('>L', f_in.read(4))[0]
    if node_type == 0:
        next_node_index = struct.unpack('>L', f_in.read(4))[0]
        opcode_id = struct.unpack('>L', f_in.read(4))[0]
        if opcode_id == 0:
            print('-----------------------CDMA-----------------------')
            count += 6 * 4
            print_info(' CDMA','node_index     ',node_index)
            print_info(' CDMA','node_type      ',node_type)
            print_info(' CDMA','next_node_index',next_node_index)
            print_info(' CDMA','opcode_id      ',opcode_id)
            for i in range (2):
                GetValue(f_in, 'CDMA', CDMA_node[i+4])

        elif opcode_id == 1:
            print('-----------------------WDMA-----------------------')
            count += 6 * 4
            print_info('WDMA','node_index          ',node_index)
            print_info('WDMA','node_type           ',node_type)
            print_info('WDMA','next_node_index     ',next_node_index)
            print_info('WDMA','opcode_id           ',opcode_id)
            for i in range (2):
                GetValue(f_in, 'WDMA', WDMA_node[i+4])

        elif opcode_id == 2:
            print('-----------------------DIDMA-----------------------')
            count += 14 * 4
            print_info('DIDMA','node_index          ',node_index)
            print_info('DIDMA','node_type           ',node_type)
            print_info('DIDMA','next_node_index     ',next_node_index)
            print_info('DIDMA','opcode_id           ',opcode_id)
            for i in range (10):
                GetValue(f_in, 'DIDMA', DIDMA_node[i+4])

        elif opcode_id == 3:
            print('-----------------------DODMA-----------------------')
            count += 13 * 4
            print_info('DODMA','node_index          ',node_index)
            print_info('DODMA','node_type           ',node_type)
            print_info('DODMA','next_node_index     ',next_node_index)
            print_info('DODMA','opcode_id           ',opcode_id)
            for i in range (9):
                GetValue(f_in, 'DODMA', DODMA_node[i+4])

        elif opcode_id == 4:
            print('-----------------------Concat-----------------------')
            
            print_info('Concat','node_index          ',node_index)
            print_info('Concat','node_type           ',node_type)
            print_info('Concat','next_node_index     ',next_node_index)
            print_info('Concat','opcode_id           ',opcode_id)
            GetValue(f_in, 'Concat', 'axis')
            input_num = struct.unpack('>L', f_in.read(4))[0]
            print_info('Concat','input_num           ',input_num)
            for i in range (input_num):
                GetValue(f_in, 'Concat', 'input_node_index_'+str(i))
            GetValue(f_in, 'Concat', 'output_node_index')              
            count += (7 + input_num) * 4
            
        elif opcode_id == 5:
            print('-----------------------PDMA-----------------------')
            count += 7 * 4
            print_info('PDMA','node_index          ',node_index)
            print_info('PDMA','node_type           ',node_type)
            print_info('PDMA','next_node_index     ',next_node_index)
            print_info('PDMA','opcode_id           ',opcode_id)
            for i in range (3):
                GetValue(f_in, 'PDMA', PDMA_node[i+4])

        elif opcode_id == 7:
            print('-----------------------LUTDMA-----------------------')
            count += 5 * 4
            print_info(' LUTDMA','node_index     ',node_index)
            print_info(' LUTDMA','node_type      ',node_type)
            print_info(' LUTDMA','next_node_index',next_node_index)
            print_info(' LUTDMA','opcode_id      ',opcode_id)
            for i in range (1):
                GetValue(f_in, 'LUTDMA', LUTDMA_node[i+4])
        else:
            print('opcode_id == ', opcode_id)
            sys.exit()

    elif node_type == 1:
        print('-----------------------NPU-----------------------')
        count += 3 * 4        
        next_node_index = struct.unpack('>L', f_in.read(4))[0]
        print_info('  NPU','node_index          ',node_index)
        print_info('  NPU','node_type           ',node_type)
        print_info('  NPU','next_node_index     ',next_node_index)

    elif node_type == 2:
        print('-----------------------datanode-----------------------')
        count += 7 * 4
        print_info('datanode','node_index          ',node_index)
        print_info('datanode','node_type           ',node_type)
        for i in range (5):
            GetValue(f_in, 'datanode', data_node[i+2])

    else:
        print('node_type == ', node_type)
        sys.exit()
  