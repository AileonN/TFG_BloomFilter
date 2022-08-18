/*
-----------------------------------------------------------
    Launch program of champsim simulations: Multi-core ReD
-----------------------------------------------------------
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

struct ChampSim_childs{
  unsigned int brainSize;
  string trace;
  int fd_pipe;
};
vector<ChampSim_childs> pipes; //Vector of Read side for the father

void waitChilds(unsigned int childs_throwed, ofstream* fcsv,unsigned int BANumber, char* SimulationInst){
    uint64_t TOTAL_ACCESS,TOTAL_HIT,TOTAL_MISS, cycles;

    cout << "Waiting simulations..." << endl;
    //Read all the results from child and then write them into the csv file
    unsigned int j=0; 
    for (auto i : pipes){
        for (int cores=0;cores<4;cores++){
          read(i.fd_pipe,&TOTAL_ACCESS,sizeof(uint64_t));read(i.fd_pipe,&TOTAL_HIT,sizeof(uint64_t));read(i.fd_pipe,&TOTAL_MISS,sizeof(uint64_t));
          read(i.fd_pipe,&cycles,sizeof(uint64_t));
          if (cores==0 && j==0){
            *fcsv << i.trace << ";" << SimulationInst;
          }
          *fcsv << ";" << TOTAL_ACCESS << ";" << TOTAL_MISS << ";" << cycles;
        }
        j++;
        if (j==2){*fcsv << endl; j=0;} 
        close(i.fd_pipe);
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
    if (argc != 7){
        cout << "./init_MultiReDSimulations warmup_inst simulation_inst file_simulations file_tracelocation file_csv num_childs" << endl;
        return 0;
    }
    char* WU_INST=argv[1]; //Warmup inst.
    char* SI_INST=argv[2]; //Simulation inst.
    string Fname=argv[3];  //File that describes the simulations to do (size of brain)
    string Ftraces=argv[4]; //File with traces names
    string Fnamecsv=argv[5]; //Name CSV file for write the results 
    MAX_NUM_CHILDS = atoi(argv[6]);

    //Read traces and simulations. Open CSV 
    string Trace, Trace1,Trace2,Trace3,Trace4, TracePath, Simulation;
    ifstream fTrace; fTrace.open(Ftraces);
    ifstream fSim; fSim.open(Fname);
    ofstream fcsv; fcsv.open("results/"+Fnamecsv);
    if (!fTrace.is_open()){
        cout << "Traces dont found" << endl;
        return 0;
    }
    if (!fSim.is_open()){
        cout << "Simulations dont found" << endl;
        return 0;
    }
    if (!fcsv.is_open()){
        cout << "CSV file isnt open" << endl;
        return 0;
    }
    //Parameters
    unsigned int nChilds = 0, NSimulation = 0;
    //Header
    fcsv << "Group;NumberInstructions";
    //Vector of Brain size 
    vector<unsigned int> BrainAddress;
    fSim >> Simulation;
    while (!fSim.eof()){
      BrainAddress.push_back(stoi(Simulation));
      for (int c=0;c<4;c++){
        fcsv << ";LLC_ACCESS_" << stoi(Simulation) << "_" << c << ";Miss_" << stoi(Simulation) << "_" << c << ";cycles_" << stoi(Simulation)
        << "_" <<c;
      }
      fSim >> Simulation;
    }
    fcsv << endl;
    if (MAX_NUM_CHILDS%BrainAddress.size() != 0){ 
      cout << "MAX_NUM_CHILDS must be multiple of BrainAddress Number" << endl; return 0; 
    }
    fTrace >> TracePath;
    getline(fTrace,Trace,';');getline(fTrace,Trace1,';');getline(fTrace,Trace2,';');getline(fTrace,Trace3,';');getline(fTrace,Trace4,'\n');
    while (!fTrace.eof()){
      string TraceParameter1 = TracePath + Trace1; 
      string TraceParameter2 = TracePath + Trace2; 
      string TraceParameter3 = TracePath + Trace3; 
      string TraceParameter4 = TracePath + Trace4.substr(0,Trace4.size()-1); 
      for(NSimulation=0; NSimulation<BrainAddress.size(); NSimulation++){
        //Create a pipe with champsim execution and save
        int phijo[2];pipe(phijo);
        ChampSim_childs c; 
        c.fd_pipe=phijo[0]; 
        c.brainSize=BrainAddress[NSimulation];
        c.trace=Trace;
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
            int nHash=1;
            if (BrainAddress[NSimulation]>0){ nHash=2;}
            char *args[] = {"bin/champsim","--warmup_instructions",WU_INST,"--simulation_instructions",SI_INST,"--traces",(char*)TraceParameter1.c_str(),(char*)TraceParameter2.c_str(),(char*)TraceParameter3.c_str(),(char*)TraceParameter4.c_str(),"--bloom_type","B"
            ,"--P","0.5","--nHash",(char*)to_string(nHash).c_str(),"--Size","1","--Max","0",
            "--Pipe",(char*)to_string(phijo[1]).c_str(),"--BrainAddress", (char*)to_string(BrainAddress[NSimulation]).c_str(), NULL};
            
            char *env[] = {"env", "LD_LIBRARY_PATH=/usr/local/gcc/lib64:/usr/local/lib",NULL};
            execve(args[0],args, env);
    
        }else if(pid==-1){ cout << "Error launching child" << endl; break;}
        //Write side -> then close
        close(phijo[1]);
        nChilds++;
        //Check max childs 
        if (nChilds==MAX_NUM_CHILDS){ 
          waitChilds(nChilds,&fcsv,BrainAddress.size(),SI_INST); nChilds=0;
        }
      }
      //NEXT Trace
      getline(fTrace,Trace,';');getline(fTrace,Trace1,';');getline(fTrace,Trace2,';');getline(fTrace,Trace3,';');getline(fTrace,Trace4,'\n');
    }
    
    //Wait for rest of childs
    if (nChilds>0){waitChilds (nChilds,&fcsv,BrainAddress.size(),SI_INST);}
    cout << "SIMULATION ENDED" << endl;
    fSim.close(); fTrace.close(); fcsv.close();
    return 0;
}


    		