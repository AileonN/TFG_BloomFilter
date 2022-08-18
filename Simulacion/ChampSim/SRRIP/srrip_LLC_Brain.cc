#include <algorithm>
#include <iterator>
#include "cache.h"
#include "util.h"
#include "brain.h"
#include <unistd.h>

#define NSET 2048
#define NWAY 16
//SRRIP
#define maxRRPV 3
//BRAIN
Brain* b;
bool Struct = false;
uint64_t Negatives = 0, Positives = 0;

void CACHE::initialize_replacement(uint16_t nHash, float P, uint32_t length, uint32_t MAX, string bloom_type, uint32_t BrainAddress) {

 if(NUM_SET==NSET && NUM_WAY==NWAY && BrainAddress != 0){ //LLC
   Struct = true;
   b = new Brain(BrainAddress);
 }
 for (auto& blk : block)
   blk.lru = maxRRPV;
}

/*
  Called when a cache fail and is full filled. Find a victim in current_set and return the victim way
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
void CACHE::update_replacement_state(uint32_t cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type,
                                     uint8_t hit)
{
  if (type == WRITEBACK){return;} 
  
  if(Struct && NUM_SET==NSET && NUM_WAY==NWAY && !hit && (type == LOAD || type == RFO || type == PREFETCH)){ //LLC load from main memory 
      
      //Search if cache isnt full filled 
      bool fullFilled = true;
      for (uint32_t i=0;i<NUM_WAY;i++){
        if ((block[set * NUM_WAY + i].address >> 6) == 0){
          fullFilled = false;
          break;
        }
      }
      //Check Brain
      bool resb = b->Find(full_addr >> 6);
      if (resb) { Positives++;} else {Negatives++;}  

      if (fullFilled){ 
        if (!resb){ 
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
void CACHE::replacement_final_stats(uint16_t nHash, float P, uint32_t length, uint32_t MAX, string bloom_type, uint32_t npipe, uint64_t TOTAL_ACCESS, uint64_t TOTAL_HIT, uint64_t TOTAL_MISS, uint64_t cycles) {
  if (NUM_SET==NSET && NUM_WAY==NWAY){ //LLC
    //Write to pipe the results
    write(npipe,&TOTAL_ACCESS,sizeof(uint64_t));write(npipe,&TOTAL_HIT,sizeof(uint64_t));write(npipe,&TOTAL_MISS,sizeof(uint64_t));
    write(npipe,&Positives,sizeof(uint64_t));write(npipe,&Negatives,sizeof(uint64_t));
	write(npipe,&cycles,sizeof(uint64_t));
  }
}
