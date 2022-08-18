import matplotlib.pyplot as plt
import numpy as np
maxDir=7 #Number of elements in axe x


def add_value_label(x_list,y_list):
    for i in range(1, len(x_list)+1):
        plt.text(i,y_list[i-1],y_list[i-1], ha="center")
        
def graph_Brain(pathfiles,filename):
    #Number of @'s, miss ratio add, number of traces
    MPKI = np.array([[0,0.0,0],[8000,0.0,0],[16000,0.0,0],[32000,0.0,0],[64000,0.0,0],[96000,0.0,0],[128000,0.0,0]]) 
    objects_axeX = [0,8000,16000,32000,64000,96000,128000]
    
    with open(pathfiles+filename,'r') as f:
        next(f) #Cabeceras
        for line in f:
            line=line.split(';')
            inst_index = 1
            miss_index = 4
            for i in range(0,maxDir):
                missRatio = float(int(line[miss_index])*1000/int(line[inst_index]))
                MPKI[i][1]+= missRatio
                MPKI[i][2]+=1
                miss_index += 5
                

    y_pos = np.arange(maxDir)
    for i in range(0,maxDir):
        print(MPKI[i][1]/MPKI[i][2])
        plt.bar(i, MPKI[i][1]/MPKI[i][2], align='center', alpha=0.5, color=['black'])
        plt.text(i,MPKI[i][1]/MPKI[i][2], round(MPKI[i][1]/MPKI[i][2],3),ha="center")
    plt.xticks(y_pos,objects_axeX)
    plt.ylabel('MPKI')
    plt.xlabel('Direcciones/bloques recordados')
    plt.title('Brain')
    plt.show()
                   
graph_Brain("results/","Brain_500M.csv")