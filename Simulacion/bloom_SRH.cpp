#include "bloom.h"

/*----------------INTERFACE CLASS BLOOM FILTER-----------------*/
BloomFilter::BloomFilter(unsigned int size, uint8_t nhash,bool debug, float P){
   
   this->size=size;
   this->P=P;
   this->nhash=nhash;
   /*Init random seeds*/
  if (!debug){
    random_device rd;							    
    mt19937_64 gen(rd());
    uniform_int_distribution<uint64_t> dist;
    for (uint8_t n=0;n<nhash;n++){
      hashseeds.push_back(dist(gen));
    }
  }else{ /* Insert static seeds (debug mode)*/
	
    for (uint8_t n=0;n<nhash;n++){
      hashseeds.push_back(hashseeds_d[n]);
    }
  }
}
vector<unsigned int> BloomFilter::GetIndexs(uint64_t Address){
  
  vector<unsigned int> indexs;

  for (uint8_t n=0;n<hashseeds.size();n++){
    /*Address with different seeds XOR*/
    uint64_t saddress = Address ^ hashseeds[n];
     /*Get hash*/
    Transformation hash; 
    MD5_Init (&hcontext);
    MD5_Update (&hcontext, &saddress, ADDRESS_LENGTH);
    MD5_Final ((unsigned char *)&hash, &hcontext);
    indexs.push_back(hash.number%size); 
  }
  return indexs;
}
bool BloomFilter::DoClean(){
   return (counter1>=(unsigned int)(size*P));
}
void BloomFilter::AddStats(string stats){
    if (stats=="TP") {s.TP++;}
    if (stats=="TN") {s.TN++;}
    if (stats=="FP") {s.FP++;}
    if (stats=="FN") {s.FN++;}
}
Stats BloomFilter::GetStats(){
  return s;
}
//DEBUG
void BloomFilter::PrintStats(){
  printf("TP:%ld FP:%ld FN:%ld TN:%ld\n",s.TP,s.FP,s.FN,s.TN);
}

unsigned int BloomFilter::getDSet(){
  return dSet;
}
unsigned int BloomFilter::getDReset(){
  return dReset;
}
/*-------------BASIC BLOOM FILTER--------------*/
Basic::Basic(unsigned int size, uint8_t nhash,bool debug, float P) :  BloomFilter(size,nhash,debug,P) {
  /*Init Basic filter*/
  vector<bool> aux (size,0);
  Storage=aux;
}
void Basic::RememberMe(vector<unsigned int> Indexs){
  for (auto i: Indexs){
    if(Storage[i]==0){Storage[i]=1; counter1++;};
  }
}
bool Basic::DoYouRememberMe(uint64_t Address, bool onlyRead){ 
  vector<unsigned int> Indexs = GetIndexs(Address);
  bool result=true;
  for (auto i: Indexs){
    if (Storage[i]==0) { if (!onlyRead) {RememberMe(Indexs);} result=false; break; }
  }
  if (DoClean()){clean();}
  return result;
}
void Basic::clean(){
  fill(Storage.begin(), Storage.end(), 0);
  counter1=0;
}
void Basic::PrintStorage(){
  cout << "State:";
  for (auto a: Storage){
    cout <<  " " << a;
  }
  cout << endl;
}
/*-------------A2 BLOOM FILTER--------------*/
A2::A2(unsigned int size, uint8_t nhash, bool debug, float P) : BloomFilter(size,nhash,debug,P){
  /*Init both filters*/
  recentS = 0; //First the recent is in the zero position
  vector<bool> aux (size,0);
  Storages.push_back(aux);Storages.push_back(aux);
}
void A2::RememberMe(vector<unsigned int> Indexs){
   for (auto i: Indexs){
    if(Storages[recentS][i]==0){Storages[recentS][i]=1; counter1++;};
  }
}
bool A2::DoYouRememberMe(uint64_t Address, bool onlyRead){ 
  vector<unsigned int> Indexs = GetIndexs(Address);
  for (auto i: Indexs){ //Check if recent remember it
    if (Storages[recentS][i]==0) { 
       bool result = true; //The recent doesnt remember it then check if the oldest does
       for (auto j: Indexs){ 
         if (Storages[!recentS][j]==0){
           result = false; break;
         }
       }
       if (!onlyRead) {RememberMe(Indexs);}
       if (DoClean()){clean(); if (!onlyRead) {RememberMe(Indexs); } }
       return result;
    }
  }
  //The recent rembember it
  return true;
}
void A2::clean(){
  fill(Storages[!recentS].begin(), Storages[!recentS].end(), 0); //The old is clean
  recentS=!recentS; //The recent now is the other
  counter1=0;
}
void A2::PrintStorage(){
  cout << "Recent State:";
  for (auto a: Storages[recentS]){
    cout <<  " " << a;
  }
  cout << endl;
  cout << "Older State: ";
  for (auto a: Storages[!recentS]){
    cout <<  " " << a;
  }
  cout << endl;
}
/*-------------SBF BLOOM FILTER--------------*/
SBF::SBF(unsigned int size, uint8_t nhash, bool debug, uint32_t MAX, float P) : BloomFilter(size,nhash,debug,P){
  this->MAX=MAX;
  vector<uint16_t> storage_aux (size,0);
  mt19937_64 gen_aux(rd());
  uniform_int_distribution<unsigned int> dist_aux(0,size-1);
  Storage = storage_aux;
  gen = gen_aux;
  dist=dist_aux; 
}
void SBF::RememberMe(vector<unsigned int> Indexs){
  //Set Indexs touched to MAX
  for (auto i : Indexs){ 
    Storage[i]=MAX;
  }
}
bool SBF::DoYouRememberMe(uint64_t Address, bool onlyRead){
  vector<unsigned int> Indexs = GetIndexs(Address);
  bool result = true;
  for (auto i : Indexs){ //Check if storage remember it
    if(Storage[i]==0){ result = false; break;}
  }
  
  if (!onlyRead) {clean(); RememberMe(Indexs);}
  return result;
}
void SBF::clean(){
  //Get P random cells and do -1
  for (unsigned int i=0;i<P;i++){
    unsigned int random = dist(gen);
    if (Storage[random]>0) { Storage[random]-=1; }
  }
}
void SBF::PrintStorage(){
    cout << "State:";
  for (auto a: Storage){
    cout <<  " " << a;
  }
  cout << endl;
}
/*-------------SR BLOOM FILTER--------------*/
SR::SR(unsigned int size, uint8_t nhash,bool debug) :  BloomFilter(size,nhash,debug,0) {
  /*Init SR filter with random 0 or 1*/
  random_device rd;							    
  mt19937_64 gen(rd());
  uniform_int_distribution<uint8_t> dist;
  vector<bool> aux (size,0);
  Storage=aux;
  for (unsigned int i=0;i<size;i++){
    Storage[i]=dist(gen)%2;
  }
}
void SR::RememberMe(vector<unsigned int> Indexs){
  bool data = Indexs[0];
  Indexs.erase(Indexs.begin());

  for (auto i: Indexs){;
    //If Set  -> write 1
    //If Reset -> write 0
    Storage[i] = data;
  }
}

bool SR::DoYouRememberMe(uint64_t Address, bool onlyRead){ 
  vector<unsigned int> Indexs = GetIndexs(Address);
  Transformation subset_sr; 
  MD5_Init (&hash_sr);
  MD5_Update (&hash_sr, &Address, ADDRESS_LENGTH);
  MD5_Final ((unsigned char *)&subset_sr, &hash_sr);
  bool odd_even = subset_sr.number & 0x1; //Even -> 0 Odd-> 1
  
  bool result=true;
  if (!odd_even){
    //Even -> Set
    dSet++;
    vector<unsigned int> Index_even;
    Index_even.push_back(1);
	//Get the first half of indexs
    for (int i=0;i<nhash/2;i++){
      Index_even.push_back(Indexs[i]);
    }
    for (int i=0;i<nhash/2;i++){
      if (Storage[Indexs[i]]==0) { if (!onlyRead) {RememberMe(Index_even);} result=false; break;}
    }
  }
  else{
    //Odd -> Reset
    dReset++;
    vector<unsigned int> Index_odd;
    Index_odd.push_back(0);
    for (int i=nhash/2;i<nhash;i++){
     Index_odd.push_back(Indexs[i]);
    }
    for (int i=nhash/2;i<nhash;i++){
      if (Storage[Indexs[i]]==1) { if (!onlyRead) {RememberMe(Index_odd);} result=false; break;}
    }
  }
  
  return result;
}
void SR::clean(){}
void SR::PrintStorage(){
  cout << "State:";
  for (auto a: Storage){
    cout <<  " " << a;
  }
  cout << endl;
}






