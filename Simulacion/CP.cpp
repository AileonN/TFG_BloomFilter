/*
----------------------------------------------
	Casos de prueba: Tablas de decisión
----------------------------------------------
*/
#include "bloom.h"
#include "brain.h"
#include <cstdlib>

int main(){
    //Casos de prueba Básico
    BloomFilter* b = new Basic(256,2,1,0.015625);
    cout << "Reuso ID 1:" << b->DoYouRememberMe(0x80000000,0) << endl;
    cout << "Estado ID 1:" << endl; b->PrintStorage();
    cout << "Reuso ID 2:" << b->DoYouRememberMe(0x80000000,0) << endl;
    cout << "Estado ID 2:" << endl; b->PrintStorage();

    cout << "Reuso ID 3:" << b->DoYouRememberMe(0x40000000,0) << endl;
    cout << "Estado ID 3:" << endl; b->PrintStorage();

    //Casos de prueba A2
    BloomFilter* ba2 = new A2(256,2,1,0.015625);
    ba2->DoYouRememberMe(0x80000000,0);
    cout << "Reuso ID 1:" << ba2->DoYouRememberMe(0x80000000,0) << endl;
    cout << "Estado ID 1:" << endl; ba2->PrintStorage();
    cout << "Reuso ID 2:" << ba2->DoYouRememberMe(0x40000000,0) << endl;
    cout << "Estado ID 2:" << endl; ba2->PrintStorage();
    ba2->DoYouRememberMe(0x10000000,0);
    cout << "Reuso ID 3:" << ba2->DoYouRememberMe(0x80000000,0) << endl;
    cout << "Estado ID 3:" << endl; ba2->PrintStorage();
    cout << "Reuso ID 4:" << ba2->DoYouRememberMe(0x80000000,0) << endl;
    cout << "Estado ID 4:" << endl; ba2->PrintStorage();

     //Casos de prueba SBF
    BloomFilter* bsbf = new SBF(256/2,2,1,3,0.015625);
    cout << "Reuso ID 1:" << bsbf->DoYouRememberMe(0x80000000,0) << endl;
    cout << "Estado ID 1:" << endl; bsbf->PrintStorage();
    cout << "Reuso ID 2:" << bsbf->DoYouRememberMe(0x80000000,0) << endl;
    cout << "Estado ID 2:" << endl; bsbf->PrintStorage();
    cout << "Reuso ID 3:" << bsbf->DoYouRememberMe(0x40000000,0) << endl;
    cout << "Estado ID 3:" << endl; bsbf->PrintStorage();
    cout << "Reuso ID 4:" << bsbf->DoYouRememberMe(0x40000000,0) << endl;
    cout << "Estado ID 4:" << endl; bsbf->PrintStorage();

    //Casos de prueba SetReset
    BloomFilter* bsr = new SR(256,2,1);
    cout << "Reuso ID 1:" << bsr->DoYouRememberMe(0x80000000,0) << endl;
    cout << "Estado ID 1:" << endl; bsr->PrintStorage();
    cout << "Reuso ID 2:" << bsr->DoYouRememberMe(0x80000001,0) << endl;
    cout << "Estado ID 2:" << endl; bsr->PrintStorage();
    cout << "Reuso ID 3:" << bsr->DoYouRememberMe(0x80000000,0) << endl;
    cout << "Estado ID 3:" << endl; bsr->PrintStorage();
    cout << "Reuso ID 4:" << bsr->DoYouRememberMe(0x80000001,0) << endl;
    cout << "Estado ID 4:" << endl; bsr->PrintStorage();

    //Casos de prueba Brain
    Brain br(4);
    cout << "Reuso ID 1:" << br.Find(0x80000000) << endl;
    cout << "Estado ID 1:" << endl;  br.PrintStorage();
    cout << "Reuso ID 2:" << br.Find(0x80000000) << endl;
    cout << "Estado ID 2:" << endl;  br.PrintStorage();
    br.Find(0x10000000);br.Find(0x20000000);br.Find(0x30000000);br.Find(0x40000000);
    cout << "Reuso ID 3:" << br.Find(0x80000000) << endl;
    cout << "Estado ID 3:" << endl;  br.PrintStorage();

    return 0;
}