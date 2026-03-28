import pandas as pd

if __name__=="__main__":

    # df = pd.read_csv('games.csv')
    with open('games.csv', 'r') as f:
        i=0
        for line in f:
            i+=1
            l = len(line.split(","))
            # if l!=39:
            print(line)

        
        print(i)

