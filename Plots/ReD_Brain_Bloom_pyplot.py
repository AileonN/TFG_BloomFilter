import matplotlib.pyplot as plt
import numpy as np
import os


markers = ['o','^','x','d','s','<','>','8','s','p','*','h','H','D','d','P','X']
#colors = ['g','r'.'m','orange']
def setPoint(x,y,marker,c,label,legend):
   
    plt.plot(x,y,marker,color=c,label=legend);
    plt.text(x,y,label)
    
def graph_BestResults(pathfiles,filename,title,x_label,y_label,x_size,y_size,font_size,metrica): 
     #Plot settings
     plt.rcParams["figure.figsize"] = (x_size,y_size) #Plot Size
     fig, ax = plt.subplots()
     ax.set_xlabel(x_label,fontsize=font_size)
     ax.set_ylabel(y_label,fontsize=font_size)
     ax.set_title(title,fontsize=font_size)
     with open(pathfiles+filename,'r') as f:
         next(f) #Headers
         m_b=0;m_sr=0;m_sbf=0;m_a2=0
         f_legend = np.array([])
         for line in f:
             
             line=line.split(';')
             name=line[0].split('-') #Name of filter 
             name_c = name[0]+"-"+name[2]+"-"+name[3]
             if name[0] == "SBF":
                 name_c = name[0]+"-"+name[3]+"-"+name[4]
             #Different filter, different color
             res=False
             
             for i in np.arange(len(f_legend)):
                 if f_legend[i]==name_c:
                     res=True
                     
                     
             if name[0]=='A2':
                 color='g'; 
                 if not res: 
                    m_a2=m_a2+1; m=markers[m_a2]
             elif name[0]=='SR':
                 color='r'; 
                 if not res: 
                     m_sr=m_sr+1; m=markers[m_sr]
             elif name[0]=='SBF':
                 color='m';  
                 if not res: 
                     m_sbf=m_sbf+1;m=markers[m_sbf]
             elif name[0]=='B':
                 color='orange'; 
                 if not res: 
                     m_b=m_b+1; m=markers[m_b]
             f_legend = np.append(f_legend, name_c)
            
             metric_value=0
             if metrica=="mpki":
                 metric_value=float(line[3].replace('\n',"").replace(',','.'))
             elif metrica=="ipc":
                 metric_value=float(line[2].replace('\n',"").replace(',','.'))
   
             #Legend   
             if name[0] == 'SR':
               nH = name[2].replace('H',"")
               nH = int(int(nH)/2)
               legend=name[0]+"-"+str(nH)+"H"
             elif name[0] == 'SBF':
               legend=name_c = name[0]+"-"+name[2]+'-'+name[3]+"-"+name[4]
             else:
               legend=name[0]+"-"+name[2]+"-"+name[3]
            
             if not res:
                 setPoint(name[1].replace("KB",""),metric_value,m,color,"",legend)
             else:
                 setPoint(name[1].replace("KB",""),metric_value,m,color,"","")
       
                 
     f.close()    
     if (metrica=="ipc"):
         plt.axhline(y=0.674288142, color = 'blue') #ReD
         plt.axhline(y=0.668858356, color = 'black') #Brain
         plt.text(0,0.674288142, 'ReD', fontsize=15, backgroundcolor='w')
         plt.text(0,0.668858356, 'Brain - 32000', fontsize=15, backgroundcolor='w')
        
     elif (metrica=="mpki"):
         plt.axhline(y=16.03658106, color = 'blue') #ReD
         plt.axhline(y=16.351, color = 'black') #Brain        
         plt.text(0,16.03658106, 'ReD', fontsize=15, backgroundcolor='w')
         plt.text(0,16.351, 'Brain - 32000', fontsize=15, backgroundcolor='w')
     handles, labels = ax.get_legend_handles_labels()
     plt.legend(bbox_to_anchor =(1, 0.4))
     plt.savefig(filename.replace(".csv","")+"_"+metrica+".png")
    
graph_BestResults("results/", "BestResults.csv", "Comparación global - MPKI", "Tamaño (KB)" ,"MPKI" , 4, 7, '15',"mpki")
graph_BestResults("results/", "BestResults.csv", "Comparación global - IPC", "Tamaño (KB)" ,"IPC" , 4, 7, '15',"ipc")

