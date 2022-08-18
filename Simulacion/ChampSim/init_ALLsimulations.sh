#!/bin/bash

 #######################
#LAUNCH ALL FINAL TEST  #
 #######################
instructions=500000000
instructions_name="500M"


                                #######################
                               #     BLOOM mono-core   #
                                #######################
./init_BloomSimulations 0 $instructions Simulations_A2-B.txt spec2k6.txt BF_A2-B_"$instructions_name" 20 32000 
./init_BloomSimulations 0 $instructions Simulations_SBF.txt spec2k6.txt BF_SBF_"$instructions_name" 55 32000 
./init_BloomSimulations 0 $instructions Simulations_SR.txt spec2k6.txt BF_SR_"$instructions_name" 10 32000 


                                #######################
                               #        BRAIN          #
                                #######################

./init_BrainSimulations 0 $instructions Simulations_Brain.txt spec2k6.txt Brain_"$instructions_name".csv 84  

                                #######################
                               #     RED mono-core     #
                                #######################		

./init_ReDSimulations 0 $instructions Simulations_ReD.txt spec2k6.txt ReDSinPC_"$instructions_name".csv 82 					
								               
instructions=50000000
instructions_name="50M"

								#######################
                               #     BLOOM multi-core  #
                                #######################
								
./init_MultiBloomSimulations 0 $instructions BestResults.txt spec2k6_multi.txt MultiBloom_"$instructions_name" 90 		
						
								#######################
                               #     RED multi-core    #
                                #######################	
								
./init_MultiReDSimulations 0 $instructions Simulations_ReD.txt spec2k6_multi.txt MultiReD_"$instructions_name".csv 90 