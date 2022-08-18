import matplotlib.pyplot as plt
import os

#--------------------------------------
#Plot: Bloom filters (IPC, MPKI, PR)
#--------------------------------------

markers = ['o','v','^','<','>','8','s','p','*','h','H','D','d','P','X']
colors = ['b','g','r','c','m','y']
def setPoint(x,y,marker,c,label,legend):
   
    plt.plot(x,y,marker,color=c,label=legend);
    plt.text(x,y,label)
    
def graph_A2_B(pathfiles,filename,title,x_label,y_label,x_size,y_size,font_size,LegendSizeBasic,metrica): 
    #Plot settings
    plt.rcParams["figure.figsize"] = (x_size,y_size) #Plot Size
    fig, ax = plt.subplots()
    ax.set_xlabel(x_label,fontsize=font_size)
    ax.set_ylabel(y_label,fontsize=font_size)
    ax.set_title(title,fontsize=font_size)
    with open(pathfiles+filename,'r') as f:
        next(f) #Cabeceras
        for line in f:

            line=line.split(';')
            name=line[0].split('-') #Name of filter
            metric_value=0
            if metrica=="acc":    
                metric_value=float(line[1].replace('\n',"").replace(',','.'))     #Accuracy
            elif metrica=="mpki":
                metric_value=float(line[3].replace('\n',"").replace(',','.'))
            elif metrica=="ipc":
                metric_value=float(line[2].replace('\n',"").replace(',','.'))
            #Default point settings
            color='red'
            marker='o'
            legend=name[2]+'-'+name[3]
            #Number of hashes -> change marker    
            if name[2]=='1H':      
                color='orange' 
            elif name[2]=='3H':
                color='green'
            elif name[2]=='7H':
                color='purple'
            #Probability of clean -> change colors
            if (name[0]=='A2' and name[3]=='35') or (name[0]=='B' and name[3]=='40'):       
                marker='^'
                #legend=legend
            elif (name[0]=='A2' and name[3]=='40') or (name[0]=='B' and name[3]=='50'):
                marker='x'
            #Legend   
            if ((name[0]=='B' or name[0]=='A2') and name[1]==LegendSizeBasic): 
                setPoint(name[1].replace("KB",""),metric_value,marker,color,"",legend)
            else:
                setPoint(name[1].replace("KB",""),metric_value,marker,color,"","")
    f.close()          
    #plt.legend(fontsize=font_size,loc='lower right') #Legend size and position
    plt.legend(bbox_to_anchor =(1, 0.5))
    plt.savefig(filename.replace(".csv","")+"_"+metrica+".png")

def graph_SBF(pathfiles,filename,title,x_label,y_label,x_size,y_size,font_size,LegendSizeSbf,metrica): 
    #Plot settings
    plt.rcParams["figure.figsize"] = (x_size,y_size) #Plot Size
    fig, ax = plt.subplots()
    ax.set_xlabel(x_label,fontsize=font_size)
    ax.set_ylabel(y_label,fontsize=font_size)
    ax.set_title(title,fontsize=font_size)
    with open(pathfiles+filename,'r') as f:
        next(f) #Cabeceras
        for line in f:

            line=line.split(';')
            name=line[0].split('-') #Name of filter
            metric_value=0
            if metrica=="acc":    
                metric_value=float(line[1].replace('\n',"").replace(',','.'))     #Accuracy
            elif metrica=="mpki":
                metric_value=float(line[3].replace('\n',"").replace(',','.'))
            elif metrica=="ipc":
                metric_value=float(line[2].replace('\n',"").replace(',','.'))
            #Default point settings
            color='red'
            marker='o'
            legend=name[3]+'-'+name[4]
            #Number of hashes -> change marker    
            if name[3]=='1H':      
                color='orange' 
            elif name[3]=='3H':
                color='green'
            #Probability of clean -> change colors
            if name[4]=='B1':       
                marker='^'
            elif name[4]=='B5':
                marker='x'
            #Legend   
            if (name[0]=='SBF' and (name[1]+"-"+name[2])==LegendSizeSbf): 
                setPoint(name[1].replace("KB","")+'-'+name[2],metric_value,marker,color,"",legend)
            else:
                setPoint(name[1].replace("KB","")+'-'+name[2],metric_value,marker,color,"","")
    
    #plt.gcf().autofmt_xdate()
    #plt.legend(fontsize=font_size,loc='upper right') #Legend size and position
    plt.legend(bbox_to_anchor =(1, 0.3))
    plt.savefig(filename.replace(".csv","")+"_"+metrica+".png")
    

def graph_SR(pathfiles,filename,title,x_label,y_label,x_size,y_size,font_size,marker,metrica): 
     #Plot settings
     plt.rcParams["figure.figsize"] = (x_size,y_size) #Plot Size
     fig, ax = plt.subplots()
     ax.set_xlabel(x_label,fontsize=font_size)
     ax.set_ylabel(y_label,fontsize=font_size)
     ax.set_title(title,fontsize=font_size)
     with open(pathfiles+filename,'r') as f:
         next(f) #Cabeceras
         i=0
         for line in f:
             
             line=line.split(';')
             name=line[0].split('-') #Name of filter             
             metric_value=0
             if metrica=="acc":    
                 metric_value=float(line[1].replace('\n',"").replace(',','.'))     #Accuracy
             elif metrica=="mpki":
                 metric_value=float(line[3].replace('\n',"").replace(',','.'))
             elif metrica=="ipc":
                 metric_value=float(line[2].replace('\n',"").replace(',','.'))
             nH = int(name[2].replace('H',""))
             nH=int(nH/2)
             #Number of hashes -> change marker    
             color_pi=colors[i]
             i=(i+1)%len(colors)
             #Legend   
             if (name[1]=="32KB"): 
                 setPoint(name[1].replace("KB",""),metric_value,marker,color_pi,"",nH)
             else:
                 setPoint(name[1].replace("KB",""),metric_value,marker,color_pi,"","")
                 
     f.close()       
     handles, labels = ax.get_legend_handles_labels()
     #ax.legend(handles[::-1], labels[::-1], bbox_to_anchor =(1, 0.3))
     plt.legend(bbox_to_anchor =(1.1, 0.2))
     plt.savefig(filename.replace(".csv","")+"_"+metrica+".png")

def average(PATH):
    
    myfiles = []
    filename = ""
    for trace in os.listdir(PATH):
        myfiles.append(open(PATH+trace))
        filename = trace.split("_")
        
        
    nline=0
    line = myfiles[0].readline()
    res = open(PATH+filename[0]+".csv", "w+")
    res.writelines("Name;Acc;IPC;MPKI\n")
    
    while line!='':
        line=line.split(";")
        if nline!=0:
            acum_acc=float(line[10])
            acum_ipc=float(line[1])/float(line[14])
            acum_mpki=1000.0*float(line[13])/float(line[1])
            
        for i in range(1,len(myfiles)):
            line_2 = myfiles[i].readline()
            line_2=line_2.split(";")
            if nline!=0:
                acum_acc+=float(line_2[10])
                acum_ipc+=float(line_2[1])/float(line_2[14])
                acum_mpki+=1000.0*float(line_2[13])/float(line_2[1])
        
        if nline!=0:
            acum_acc = str(acum_acc/len(myfiles))
            acum_ipc = str(acum_ipc/len(myfiles))
            acum_mpki = str(acum_mpki/len(myfiles))
            res.writelines(line[0]+";"+acum_acc.replace('.',',')+";"+acum_ipc.replace('.',',')+";"+acum_mpki.replace('.',',')+"\n")
        nline+=1
        line = myfiles[0].readline()        
    res.close()
    return filename[0]
        
def main(): 
    PATH = "results/"
    bf_type=average(PATH)
    if bf_type=="B":
        graph_A2_B(PATH,bf_type+".csv","Básico extendido - PR","Tamaño (KB)","PR",5,7,'15',"32KB","acc")
        graph_A2_B(PATH,bf_type+".csv","Básico extendido - MPKI","Tamaño (KB)","MPKI",5,7,'15',"32KB","mpki")
        graph_A2_B(PATH,bf_type+".csv","Básico extendido - IPC","Tamaño (KB)","IPC",5,7,'15',"32KB","ipc")
    elif bf_type=="A2":
        graph_A2_B(PATH,bf_type+".csv","A2 - PR","Tamaño (KB)","PR",5,7,'15',"32KB","acc")
        graph_A2_B(PATH,bf_type+".csv","A2 - MPKI","Tamaño (KB)","MPKI",5,7,'15',"32KB","mpki")
        graph_A2_B(PATH,bf_type+".csv","A2 - IPC","Tamaño (KB)","IPC",5,7,'15',"32KB","ipc")
    elif bf_type=="SBF":
        graph_SBF(PATH,bf_type+".csv","SBF - PR","Tamaño (KB)","PR",10,15,'15',"32KB-1b","acc")
        graph_SBF(PATH,bf_type+".csv","SBF - MPKI","Tamaño (KB)","MPKI",10,15,'15',"32KB-1b","mpki")
        graph_SBF(PATH,bf_type+".csv","SBF - IPC","Tamaño (KB)","IPC",10,15,'15',"32KB-1b","ipc")
    elif bf_type=="SR":
        graph_SR(PATH,bf_type+".csv","SR - PR","Tamaño (KB)","PR",5,7,'15',markers[0],"acc")
        graph_SR(PATH,bf_type+".csv","SR - MPKI","Tamaño (KB)","MPKI",5,7,'15',markers[0],"mpki")
        graph_SR(PATH,bf_type+".csv","SR - IPC","Tamaño (KB)","IPC",5,7,'15',markers[0],"ipc")
    
main()

