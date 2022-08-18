/*
----------------------------------------------------------
    Launch program of champsim simulations: Bloom filters
----------------------------------------------------------
*/

#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <math.h>
#include <cstring>
using namespace std;

 
uint32_t MAX_NUM_CHILDS; //Max number of childs at the same time

struct Champ_Childs{
    string bf_name; //Name of the simulation
    int fd_pipe;    //Read side for the father
};
vector<Champ_Childs> pipes; //Vector of champ_childs

void waitChilds(unsigned int childs_throwed, ofstream* fcsv, uint64_t SimInst){
    uint64_t TP,FP,FN,TN,TOTAL_ACCESS,TOTAL_HIT,TOTAL_MISS, cycles, num_access;
    cout << "Waiting simulations..." << endl;
    //Read all the results from child and then write them into the csv file
    for (auto i : pipes){ 
        read(i.fd_pipe,&TOTAL_ACCESS,sizeof(uint64_t));read(i.fd_pipe,&TOTAL_HIT,sizeof(uint64_t));read(i.fd_pipe,&TOTAL_MISS,sizeof(uint64_t));
        read(i.fd_pipe,&cycles,sizeof(uint64_t));
        read(i.fd_pipe,&TP,sizeof(uint64_t));read(i.fd_pipe,&FP,sizeof(uint64_t));
        read(i.fd_pipe,&FN,sizeof(uint64_t));read(i.fd_pipe,&TN,sizeof(uint64_t));
        close(i.fd_pipe);
        num_access = TP+TN+FP+FN;
        if (num_access != 0){
            *fcsv << i.bf_name << ";" << SimInst << ";" << TP << ";" << FP << ";" << FN << ";" << TN << ";"
            << (100*TP)/num_access << ";" << (100*FP)/num_access << ";"  << (100*FN)/num_access << ";" << (100*TN)/num_access  << ";"
            << (float)(TP+TN)/(float)(TP+TN+FP+FN) << ";" << TOTAL_ACCESS << ";" <<  TOTAL_HIT << ";" << TOTAL_MISS << ";" << cycles << endl;
        }else{
            *fcsv << i.bf_name << ";" << SimInst << ";" << TP << ";" << FP << ";" << FN << ";" << TN << ";"
            << "0;0;0;0;0.0" << ";" << TOTAL_ACCESS << ";" <<  TOTAL_HIT << ";" << TOTAL_MISS << ";" << cycles << endl;
        }
    } 
    pipes.clear();
    //Wait for end process
    pid_t pid;
    for (unsigned int i=0;i<childs_throwed;i++){
        while ((pid = wait(NULL)) > 0) {
    			cout << "Process " <<pid << " finished" << endl;
    	}
    }
    cout << "Packet of simulations finished" << endl;
}
int main(int argc, char** argv){
    //Check arguments
    if (argc != 8){
        cout << "./init_BloomSimulations warmup_inst simulation_inst file_simulations file_tracelocation file_csv num_childs BrainAddress" << endl;
        return 0;
    }
    char* WU_INST=argv[1]; //Warmup inst.
    char* SI_INST=argv[2]; //Simulation inst.
    string Fname=argv[3];  //File that describes the simulations to do
    string Ftraces=argv[4]; //File with traces names
    string Fnamecsv_init=argv[5]; //Name CSV file for write the results 
    MAX_NUM_CHILDS = atoi(argv[6]);
    uint32_t BrainAddress = atoi(argv[7]);

    string Trace, TracePath;
    ifstream fTrace;
    fTrace.open(Ftraces);
    if (!fTrace.is_open()){
        cout << "Traces dont found" << endl;
        return 0;
    }
    fTrace >> TracePath;
    fTrace >> Trace;
        
    while (!fTrace.eof()){
        
        string Fnamecsv=Fnamecsv_init+Trace.substr(0,Trace.size()-3)+".csv";
        Trace = TracePath + Trace;
        //Open both files
        ifstream f; ofstream fcsv;
        f.open(Fname); fcsv.open("results/"+Fnamecsv);
        if (f.is_open() && fcsv.is_open()){
            //Cabecera
            fcsv << "Name;NumberInstructions;TP;FP;FN;TN;%TP;%FP;%FN;%TN;Accuracy;LLC_TOTAL_ACCESS;LLC_TOTAL_HIT;LLC_TOTAL_MISS;cycles" << endl;
            //Parameters
            string line,bloom_type, desc;
            int entries,nHash,max,bitspercell;
            float P;
            unsigned int nChilds = 0;
            //Read simulations file
            f >> line;
            while(!f.eof()){
                //Read type of simulation
                desc=line;
                bloom_type=line.substr(0,line.find('-')); line.erase(0, line.find('-') + 1);
                entries=stoi(line.substr(0,line.find('K'))); entries=entries*1024*8; //Entries of 1 bit size
                max=0; 
                if (bloom_type=="SBF"){
                    bitspercell=stoi(line.substr(line.find('-')+1,line.find('b')));  //Reads bits x cell
                    entries=entries/bitspercell;
                    max=pow(2,bitspercell)-1; //Convert to max value
                    line.erase(0, line.find('-') + 1);
                }else if (bloom_type=="A2"){
                    entries=entries/2; //Size of both tables = entries -> 1 table has the half of entries
                }
                line.erase(0, line.find('-') + 1);
                nHash=stoi(line.substr(0,line.find('-')));
                if (bloom_type=="SBF"){ line.erase(0, line.find('B') + 1);} else{ line.erase(0, line.find('-') + 1);}
                P=stof(line.substr(0,line.find('\n'))); 
                if (bloom_type!="SBF"){P=P/100.0;}

                //Create a pipe with champsim execution and save
                int phijo[2];pipe(phijo);
                Champ_Childs c; c.fd_pipe=phijo[0]; c.bf_name=desc;
                pipes.push_back(c);

                //Throw child
                pid_t pid = 0;
                pid = fork();
                if (pid==0){ //Exec with the simulation

                    //Redirect stdout to /dev/null  
                    FILE* f_c = fopen("/dev/null","w");
                    int fd_c = fileno(f_c);
                    if (fd_c == -1) { cout << "Error in /dev/null" << endl; return 0;} 
                    dup2(fd_c,1);close(fd_c);
                    //Pipe for results, close read side 
                    close(phijo[0]);

                    char *args[] = {"bin/champsim","--warmup_instructions",WU_INST,"--simulation_instructions",SI_INST,(char*)Trace.c_str(),"--bloom_type",&bloom_type[0]
                    ,"--P",(char*)to_string(P).c_str(),"--nHash",(char*)to_string(nHash).c_str(),"--Size",(char*)to_string(entries).c_str(),"--Max",(char*)to_string(max).c_str(),
                    "--Pipe",(char*)to_string(phijo[1]).c_str(),"--BrainAddress", (char*)to_string(BrainAddress).c_str(), NULL};
                    
                    char *env[] = {"env", "LD_LIBRARY_PATH=/usr/local/gcc/lib64:/usr/local/lib",NULL};
                    execve(args[0],args, env);

                }else if(pid==-1){ cout << "Error launching child" << endl; break;}
                //Write side -> then close
                close(phijo[1]);
                nChilds++;

                //Check max childs 
                if (nChilds==MAX_NUM_CHILDS){ waitChilds(nChilds,&fcsv,stoi(SI_INST)); nChilds=0;}
                f >> line;
            }
            
            //Wait for rest of childs
            if (nChilds>0){waitChilds (nChilds,&fcsv,stoi(SI_INST));}
            cout << "SIMULATION ENDED" << endl;
            f.close(); fcsv.close();
        }else{
            cout << "File not found" << endl;
        }
        //NEXT Trace
        fTrace >> Trace;
    }
    return 0;
}


    		