import pandas as pd
import math

def round_up_pow2(v: int) -> int:
    if v <= 1:
        return 1
    return 1 << ((v - 1).bit_length())


def round_up_pow2_exp(v: int) -> int:
    if v <= 1:
        return 0  # 2^0 = 1
    return (v - 1).bit_length()

if __name__=="__main__":

    #df = pd.read_csv('games.csv')
    # with open('games.csv', 'r') as f:
    #     i=0
    #     for line in f:
    #         i+=1
    #         l = len(line.split(","))
    #         # if l!=39:
    #         print(line)

        
    #     print(i)

    

    x = round_up_pow2(2)

    for i in range(50):
        
        print(math.pow(2, i)/(1024*1024))
