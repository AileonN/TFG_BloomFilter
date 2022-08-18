#include <algorithm>
#include <iterator>
#include "cache.h"
#include "util.h"
#include "brain.h"
#include "bloom.h"
#include <unistd.h>

#define NSET 8192
#define NWAY 16
#define NUMCORES 4
//SRRIP
#define maxRRPV 3
//BLOOM
BloomFilter* bf[NUMCORES];

void CACHE::initialize_replacement(uint16_t nHash, float P, uint32_t length, uint32_t MAX, string bloom_type, uint32_t BrainAddress) {
 if(NUM_SET==NSET && NUM_WAY==NWAY){ //LLC
  /*Initialize the filter, default debug mode */
    if (bloom_type=="B"){
        for (int c=0;c<NUMCORES;c++){
          bf[c] = new Basic(length,nHash,0,P);
        }
    }else if (bloom_type=="A2"){
      for (int c=0;c<NUMCORES;c++){
          bf[c] = new A2(length,nHash,0,P);
      }
    }else if (bloom_type=="SBF"){
      for (int c=0;c<NUMCORES;c++){
          bf[c] = new SBF(length,nHash,0,MAX,P); 
      }
    }else{
      for (int c=0;c<NUMCORES;c++){
          bf[c] = new SR(length,nHash,0);
      }
    }
 }
 for (auto& blk : block)
   blk.lru = maxRRPV;
}

/*
  Called when there is a fail in cache and is also full filled. Find a victim in current_set and return the victim way
*/
uint32_t CACHE::find_victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
  // look for the maxRRPV line
  auto begin = std::next(std::begin(block), set * NUM_WAY);
  auto end = std::next(begin, NUM_WAY);
  //Look for bypass
  auto victim = std::find_if(begin, end, [](BLOCK x) { return x.lru == maxRRPV + 1; }); 
  if (victim == end) { 
    //No Bypass -> search maxRRPV
    victim = std::find_if(begin, end, [](BLOCK x) { return x.lru == maxRRPV; });
    while (victim == end) {
      for (auto it = begin; it != end; ++it) {it->lru++;}
      victim = std::find_if(begin, end, [](BLOCK x) { return x.lru == maxRRPV; });
    }
  }
  return std::distance(begin, victim);
}

/*
 Called when:
  - Fail but cache isnt full filled. There is no replacement.
  - Hit
  Update block state.
*/
void CACHE::update_replacement_state(uint32_t cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
  if (type == WRITEBACK){return;} 
  
  if(NUM_SET==NSET && NUM_WAY==NWAY && !hit && (type == LOAD || type == RFO || type == PREFETCH)){ //LLC load from main memory 
      
      //Search if cache isnt full filled 
      bool fullFilled = true;
      for (uint32_t i=0;i<NUM_WAY;i++){
        if ((block[set * NUM_WAY + i].address >> 6) == 0){
          fullFilled = false;
          break;
        }
      }
      //Check Brain
      bool resbf = bf[cpu]->DoYouRememberMe(full_addr >> 6,0); 

      if (fullFilled){ 
        if (!resbf && current_cycle%32!=0){ 
		    /* "BYPASS" */
		      block[set * NUM_WAY + way].lru = maxRRPV + 1;
          return; 
        }
      }
  }  

  if (hit)
	 block[set * NUM_WAY + way].lru = 0;
  else
	 block[set * NUM_WAY + way].lru = maxRRPV - 1;
}
/*
  Called at the end of the program
*/
void CACHE::replacement_final_stats() {
}
