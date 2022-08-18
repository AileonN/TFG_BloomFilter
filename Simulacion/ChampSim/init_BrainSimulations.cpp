/*
--------------------------------------------------
    Launch program of champsim simulations: Brain
--------------------------------------------------
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
    uint64_t TOTAL_ACCESS,TOTAL_HIT,TOTAL_MISS, Positives, Negatives,cycles;

    cout << "Waiting simulations..." << endl;
    //Read all the results from child and then write them into the csv file
    unsigned int j=0; 
    for (auto i : pipes){ 
        read(i.fd_pipe,&TOTAL_ACCESS,sizeof(uint64_t));read(i.fd_pipe,&TOTAL_HIT,sizeof(uint64_t));read(i.fd_pipe,&TOTAL_MISS,sizeof(uint64_t));
        read(i.fd_pipe,&Positives,sizeof(uint64_t));read(i.fd_pipe,&Negatives,sizeof(uint64_t)); read(i.fd_pipe,&cycles,sizeof(uint64_t));
        close(i.fd_pipe);
        if (j==BANumber-1){ //Next trace
          j=0;
          *fcsv << ";" << TOTAL_ACCESS << ";" << cycles << ";" << TOTAL_MISS << ";" << Positives << ";" << Negatives << endl;
        }else if (j==0){ //New trace
          j++;
          *fcsv << i.trace << ";" << stoi(SimulationInst) << ";" << TOTAL_ACCESS << ";" << cycles << ";" << TOTAL_MISS << ";" << Positives << ";" << Negatives;
        }else{ //Data from trace
          j++;
          *fcsv << ";" << TOTAL_ACCESS << ";" << cycles << ";" << TOTAL_MISS << ";" << Positives << ";" << Negatives;
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
    if (argc != 7){
        cout << "./init_BrainSimulations warmup_inst simulation_inst file_simulations file_tracelocation file_csv num_childs" << endl;
        return 0;
    }
    char* WU_INST=argv[1]; //Warmup inst.
    char* SI_INST=argv[2]; //Simulation inst.
    string Fname=argv[3];  //File that describes the simulations to do (size of brain)
    string Ftraces=argv[4]; //File with traces names
    string Fnamecsv=argv[5]; //Name CSV file for write the results 
    MAX_NUM_CHILDS = atoi(argv[6]);

    //Read brain traces and simulations. Open CSV 
    string Trace, TracePath, Simulation;
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
    fcsv << "Trace;NumberInstructions";
    //Vector of Brain size 
    vector<unsigned int> BrainAddress;
    fSim >> Simulation;
    while (!fSim.eof()){
      BrainAddress.push_back(stoi(Simulation));
      fcsv << ";LLC_ACCESS_" << stoi(Simulation) << ";cycles" << stoi(Simulation) << ";Miss_" << stoi(Simulation) << ";Positives_" << stoi(Simulation) << ";Negatives_" << stoi(Simulation);
      fSim >> Simulation;
    }
    fcsv << endl;
    if (MAX_NUM_CHILDS%BrainAddress.size() != 0){ 
      cout << "MAX_NUM_CHILDS must be multiple of BrainAddress Number" << endl; return 0; 
    }
    fTrace >> TracePath;
    fTrace >> Trace;
    while (!fTrace.eof()){
      string TraceParameter = TracePath + Trace;
      for(NSimulation=0; NSimulation<BrainAddress.size(); NSimulation++){
        //Create a pipe with champsim execution and save
        int phijo[2];pipe(phijo);
        ChampSim_childs c; 
        c.fd_pipe=phijo[0]; 
        c.brainSize=BrainAddress[NSimulation];
        c.trace=Trace.substr(0,Trace.size()-3);
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
            
            char *args[] = {"bin/champsim","--warmup_instructions",WU_INST,"--simulation_instructions",SI_INST,(char*)TraceParameter.c_str(),"--bloom_type","B"
            ,"--P","0.5","--nHash","1","--Size","1","--Max","0",
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
      fTrace >> Trace;
    }
    
    //Wait for rest of childs
    if (nChilds>0){waitChilds (nChilds,&fcsv,BrainAddress.size(),SI_INST);}
    cout << "SIMULATION ENDED" << endl;
    fSim.close(); fTrace.close(); fcsv.close();
    return 0;
}


    		