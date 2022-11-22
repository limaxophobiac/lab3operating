#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


int main(int argc, char *argv[]){

    FILE *fAddresses, *fBackStore;

    int8_t (*physMem)[256] = malloc(256*256);
    int physSpace = 0, tlbCounter = 0, physAddress, tlbMissess = 0, count = 0;
    bool tlbHit = false;
    int pageTable[256];
    int TLB[16][2];
    for (int i = 0; i < 16; i++)
        TLB[16][0] = -1;
    for (int i = 0; i < 256; i++)
        pageTable[i] = -1;

    if (argc != 2){
        printf("usage: a.out filename");
        exit(EXIT_FAILURE);
    }

    if ((fAddresses = fopen(argv[1], "r")) == NULL){
        printf("could not open file %s", argv[1]);
        exit(EXIT_FAILURE);
    }

    if ((fBackStore= fopen("BACKING_STORE.bin", "rb")) == NULL){
        printf("could not open file %s", "BACKING_STORE.bin");
        exit(EXIT_FAILURE);
    }

    u_int32_t address, pageNr, offset;
    while (fscanf(fAddresses, "%u", &address) == 1){
        count++;
        tlbHit = false;
        pageNr = (address & 0x0000ff00) >> 8;
        offset = address & 0x000000ff;
        
        for (int i = 0; i < 16; i++){
            if (TLB[i][0] == pageNr){ 
                tlbHit = true;
                physAddress = TLB[i][1]*256 + offset;
                break;
            }
        }

        //handle TLB miss
        if (!tlbHit){
            tlbMissess++;
            //handle page fault by loading from simulated BackingStore to memory 
            if (pageTable[pageNr] == -1){
                fseek(fBackStore, (long int) pageNr*256, SEEK_SET);
                for (int i = 0; i < 256; i++)
                    physMem[physSpace][i] = fgetc(fBackStore);
                pageTable[pageNr] = physSpace++;
            }
            TLB[tlbCounter][0] = pageNr;
            TLB[tlbCounter][1] = pageTable[pageNr];
            tlbCounter = (tlbCounter + 1) % 16;

            physAddress = pageTable[pageNr]*256 + offset;
        }
        printf("Virtual address: %u Physical address: %d Value: %hhd\n", address, physAddress, physMem[pageTable[pageNr]][offset]);
    }

    printf("Page fault rate: %.1f%% TLB hit-rate: %.1f%%\n", 100* (double) physSpace / count, 100* (double) (count - tlbMissess)/ count);
    free(physMem);
    fclose(fAddresses);
    fclose(fBackStore);
    return 0;
}