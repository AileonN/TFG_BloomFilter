#include "brain.h"

Element StorageBrain[N][M] = {0}; //NxM (MAX 2^64 * 2^31)

Brain::Brain(uint32_t BrainAddress){
  /*Init the number of elements in a row */
  vector<uint64_t> aux (N,0);
  elementsInRow=aux; //0 - M-1
  MAX_TIMESTAMP = BrainAddress;
};
bool Brain::Find(uint64_t Address){

    global_timestamp++;
    /*Find with hash in the first vector*/
    uint64_t index;
    /*Get index with hashing*/
    Transformation hash; 
    MD5_Init (&hcontext);
    MD5_Update (&hcontext, &Address, 8);
    MD5_Final ((unsigned char *)&hash, &hcontext);
    index = hash.number%(uint64_t)N; 
    /*Find the address in StorageBrain*/
    int i = elementsInRow[index] - 1; 
    for (; i>=0 && i<M;i--){
        if (StorageBrain[index][i].address==Address){
            bool result = true;
            if ((global_timestamp - StorageBrain[index][i].timestamp) > MAX_TIMESTAMP){
                //If the address was referenced MAX_TIMESTAMP @ or more ago, brain tells that the @ isnt here
                result = false;
                StorageBrain[index][i].timestamp = global_timestamp;  //Refresh timestamp 
            }
            
            return result;
        }
    }
	//Its not in the Brain so we have to save it 
    if (elementsInRow[index]==M) {  
		//No space left
        ofstream f;
        f.open("ERROR");
        f << "Need more space" << endl;
        f.close();
    }
    else {
        StorageBrain[index][elementsInRow[index]].address = Address; 
        StorageBrain[index][elementsInRow[index]].timestamp = global_timestamp;
        elementsInRow[index]++;
    }
    return false;
}
void Brain::PrintStorage(){
    for (uint64_t i = 0; i<N ; i++){
        printf("Fila %ld ",i);
        cout << "Columnas "; 
        for (uint64_t j=0; j<M; j++){
           printf("%lx %ld ", StorageBrain[i][j].address,StorageBrain[i][j].timestamp);
        }
        cout << endl;
  }
}
void Brain::WriteStorage(string filename){
  cout << "Escribiendo..." << endl;
  ofstream f;
  f.open(filename);
  for (uint64_t i = 0; i<N ; i++){
      f << "Fila " << i << " :";
      for (uint64_t j=0; j<M; j++){
          f << " " << StorageBrain[i][j].address << " timestamp: " << StorageBrain[i][j].timestamp;
      }
      f << endl;
  }
  f.close();
  cout << "Fin escritura" << endl;
}