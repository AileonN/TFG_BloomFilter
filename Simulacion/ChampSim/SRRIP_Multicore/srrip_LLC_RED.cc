#include "cache.h"
#include "brain.h"
#include <fstream>
#include <unistd.h>

#define NUM_CORE 4
#define LLC_SETS 2048*NUM_CORE
#define LLC_WAYS 16

#define RED_SETS_BITS 9
#define RED_WAYS 16
#define RED_TAG_SIZE_BITS 11
#define RED_SECTOR_SIZE_BITS 2
#define RED_PC_FRACTION 4
#define PCs_BITS 8

#define RED_SETS (1<<RED_SETS_BITS)
#define RED_TAG_SIZE (1<<RED_TAG_SIZE_BITS)
#define RED_SECTOR_SIZE (1<<RED_SECTOR_SIZE_BITS)
#define PCs (1<<PCs_BITS)
//SRRIP
#define maxRRPV 3
// ReD 

struct ReD_ART_bl {
	uint64_t tag;                       // RED_TAG_SIZE_BITS
	uint8_t valid[RED_SECTOR_SIZE];     // 1 bit
};

struct ReD_ART_set {
	struct ReD_ART_bl adds[RED_WAYS];
	uint32_t insert;                    // 4 bits, log2(RED_WAYS)
};

struct ReD_ART_set ReD_ART[NUM_CORE][RED_SETS];

struct ReD_ART_PCbl {
	uint64_t pc_entry[RED_SECTOR_SIZE];  // PCs_BITS
};

struct ReD_ART_PCbl ReD_ART_PC[NUM_CORE][RED_SETS/RED_PC_FRACTION][RED_WAYS];

struct ReD_PCRT_bl {
	uint32_t reuse_counter,            // 10-bit counter
			 noreuse_counter;              // 10-bit counter
};

struct ReD_PCRT_bl ReD_PCRT[NUM_CORE][PCs];

uint32_t misses[NUM_CORE];
//Brain
bool Struct = false;

// search for an address in ReD_ART
uint8_t lookup(uint32_t cpu, uint64_t PCnow, uint64_t block)
{
	uint64_t PCin_entry;
	uint64_t i, tag, subsector;
	uint64_t ART_set;
	
	subsector=block & (RED_SECTOR_SIZE-1);
	ART_set = (block>>RED_SECTOR_SIZE_BITS) & (RED_SETS -1);
	tag=(block>>(RED_SETS_BITS+RED_SECTOR_SIZE_BITS)) & (RED_TAG_SIZE-1);

	misses[cpu]++;
	
	for (i=0; i<RED_WAYS; i++) {
		if ((ReD_ART[cpu][ART_set].adds[i].tag == tag) && (ReD_ART[cpu][ART_set].adds[i].valid[subsector] == 1)) {
			if (ART_set % RED_PC_FRACTION == 0) {
				// if ART set stores PCs, count the reuse in PCRT
				PCin_entry = ReD_ART_PC[cpu][ART_set/RED_PC_FRACTION][i].pc_entry[subsector];
				//ReD_PCRT[cpu][PCin_entry].reuse_counter++;

				if (ReD_PCRT[cpu][PCin_entry].reuse_counter > 1023) {
					// 10-bit counters, shift when saturated
					//ReD_PCRT[cpu][PCin_entry].reuse_counter>>=1;
				  //ReD_PCRT[cpu][PCin_entry].noreuse_counter>>=1;
				}
				// Mark as invalid to count only once
				ReD_ART[cpu][ART_set].adds[i].valid[subsector] = 0; 
			}
			// found
			return 1;
		}
	}

	// not found
	return 0;
}

// remember a block in ReD_ART
void remember(uint32_t cpu, uint64_t PCnow, uint64_t block)
{
	uint32_t where;
	uint64_t i, tag, subsector, PCev_entry, PCnow_entry;
	uint64_t ART_set;
	
	subsector=block & (RED_SECTOR_SIZE-1);
	ART_set = (block>>RED_SECTOR_SIZE_BITS) & (RED_SETS -1);
	tag=(block>>(RED_SETS_BITS+RED_SECTOR_SIZE_BITS)) & (RED_TAG_SIZE-1);

	PCnow_entry = (PCnow >> 2) & (PCs-1);

	// Look first for the tag in my set
	for (i=0; i<RED_WAYS; i++) {
		if (ReD_ART[cpu][ART_set].adds[i].tag == tag)
			break;
	}
	
	if (i != RED_WAYS) {
		// Tag found, remember in the specific subsector
		ReD_ART[cpu][ART_set].adds[i].valid[subsector] = 1;

		if (ART_set % RED_PC_FRACTION == 0) {
			ReD_ART_PC[cpu][ART_set/RED_PC_FRACTION][i].pc_entry[subsector] = PCnow_entry;
		}
	}
	else {
		// Tag not found, need to replace entry in ART
		where = ReD_ART[cpu][ART_set].insert;
		
		if (ART_set % RED_PC_FRACTION == 0) {
			// if ART set stores PCs, count noreuse of evicted PCs if needed
			for (int s=0; s<RED_SECTOR_SIZE; s++) {
				if (ReD_ART[cpu][ART_set].adds[where].valid[s]) {
					PCev_entry = ReD_ART_PC[cpu][ART_set/RED_PC_FRACTION][where].pc_entry[s];
					//ReD_PCRT[cpu][PCev_entry].noreuse_counter++;

					// 10-bit counters, shift when saturated
					if (ReD_PCRT[cpu][PCev_entry].noreuse_counter > 1023) {
						//ReD_PCRT[cpu][PCev_entry].reuse_counter>>=1;
						//ReD_PCRT[cpu][PCev_entry].noreuse_counter>>=1;
					}
				}
			}
		}
		
		// replace entry to store new block address
		
		ReD_ART[cpu][ART_set].adds[where].tag = tag;
		for (int j=0; j<RED_SECTOR_SIZE; j++) {
			ReD_ART[cpu][ART_set].adds[where].valid[j] = 0;
		}
		ReD_ART[cpu][ART_set].adds[where].valid[subsector] = 1;
		
		if (ART_set % RED_PC_FRACTION == 0) {
			ReD_ART_PC[cpu][ART_set/RED_PC_FRACTION][where].pc_entry[subsector] = PCnow_entry;
		}
		
		// update pointer to next entry to replace
		ReD_ART[cpu][ART_set].insert++;
		if (ReD_ART[cpu][ART_set].insert == RED_WAYS) 
			ReD_ART[cpu][ART_set].insert = 0;
	}
}

// initialize replacement state
void CACHE::initialize_replacement(uint16_t nHash, float P, uint32_t length, uint32_t MAX, string bloom_type, uint32_t BrainAddress)
{
  // ReD init
  if (NUM_SET==LLC_SETS && NUM_WAY==LLC_WAYS && BrainAddress != 0){
    Struct = true;
    cerr <<"AAAAAAAAAAAAAAAAAAAAAAAAAAAA" << endl;
    // ReD init
  	for (int core=0; core<NUM_CORE; core++) {
  		for (int i=0; i<RED_SETS; i++) {
  			ReD_ART[core][i].insert = 0;
  			for (int j=0; j<RED_WAYS; j++) {
  				ReD_ART[core][i].adds[j].tag=0;
  				for (int k=0; k<RED_SECTOR_SIZE; k++) {
  					ReD_ART[core][i].adds[j].valid[k] = 0;
  				}
  			}
  		}
  	}
  
  	for (int core=0; core<NUM_CORE; core++) {
  		for (int i=0; i<RED_SETS/RED_PC_FRACTION; i++) {
  			for (int j=0; j<RED_WAYS; j++) {
  				for (int k=0; k<RED_SECTOR_SIZE; k++) {
  					ReD_ART_PC[core][i][j].pc_entry[k] = 0;
  				}
  			}
  		}
  	}
  
  	for (int core=0; core<NUM_CORE; core++) {
  		for (int i=0; i<PCs; i++) {
  			ReD_PCRT[core][i].reuse_counter = 3;
  			ReD_PCRT[core][i].noreuse_counter = 10;
  		}
  	}
  
  	for (int i=0; i<NUM_CORE; i++) {
  	   misses[i]=0; 
  	}
  }
    // initialize replacement state
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
  // Write-backs do not change rrpv
	if (type == WRITEBACK) return; 
 
  if (Struct && !hit && NUM_WAY == LLC_WAYS && NUM_SET == LLC_SETS && (type == LOAD || type == RFO || type == PREFETCH)){
      
	  //Search if cache isnt full filled 
      bool fullFilled = true;
      for (uint32_t i=0;i<NUM_WAY;i++){
        if ((block[set * NUM_WAY + i].address >> 6) == 0){
          fullFilled = false;
          break;
        }
      }
	  
	  uint8_t present;
      uint64_t PCentry;
      uint64_t block_address;
      
      block_address = full_addr >> 6; // assuming 64B line size, get rid of lower bits
      PCentry = (ip >> 2) & (PCs-1);
 
      present = lookup(cpu, ip, block_address);		  
      if (!present) {
        // Remember in ART only if reuse in PCRT is intermediate, or one out of eight times
        if (   (    ReD_PCRT[cpu][PCentry].reuse_counter * 64 > ReD_PCRT[cpu][PCentry].noreuse_counter
        	 && ReD_PCRT[cpu][PCentry].reuse_counter * 3 < ReD_PCRT[cpu][PCentry].noreuse_counter)
        || (misses[cpu] % 8 == 0)) 
        {
          remember(cpu, ip, block_address);
        }
        // bypass when address not in ART and reuse in PCRT is low or intermediate
        if (fullFilled && current_cycle%32!=0 && ReD_PCRT[cpu][PCentry].reuse_counter * 3 < ReD_PCRT[cpu][PCentry].noreuse_counter) {
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
