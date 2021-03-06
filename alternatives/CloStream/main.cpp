// Moment.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "../../generic_Transaction.h"
#include "CloStream.h"

#include <cstdio>   //fopen, printf
#include <cstdlib>  //atol
#include <queue>    //sliding window
#include <cstring>  //strtok
#include <ctime>    //clock_t
#include <iostream> //cout
#include <map>
#include <vector>
#include <set>


#ifdef _WIN32
//#include <windows.h>
//#include <psapi.h>
#endif


extern std::map<uint32_t, std::queue<uint32_t>> POSITIONS_QUEUES_INDEX;

int main(int argc, char *argv[]) {
  if (argc != 4) return 0;
  clock_t start = clock(); clock_t running = clock();
  std::queue<Transaction<uint32_t>> window;
  //uint32_t window_size = 50;
  const uint32_t window_size = strtoul(argv[1], 0, 10);//1500
  const uint32_t MAX_ATTRIBUTES = strtoul(argv[2], 0, 10);//100001
  const uint32_t minsup = strtoul(argv[3], 0, 10);//1
  //const uint32_t MAX_ATTRIBUTES = 1001;
  std::map<uint32_t, CloStreamCI*> tableClosed = std::map<uint32_t, CloStreamCI*>();
  std::map<uint32_t, std::vector<uint32_t>> cidListMap = std::map<uint32_t, std::vector<uint32_t>>();
  CloStreamCI emptySet = CloStreamCI();
  emptySet.support = 0;
  emptySet.id = 0;
  emptySet.itemset = new std::vector<uint32_t>();
  // add the empty set in the list of closed sets
  tableClosed.emplace(0, &emptySet);

  for (uint32_t i = 0; i != MAX_ATTRIBUTES; ++i) {
    cidListMap.emplace(i, std::vector<uint32_t>());
    POSITIONS_QUEUES_INDEX.emplace(i, std::queue<uint32_t>());
  }

  //238709

  char s[10000];
  uint32_t i = 0;
  while (fgets(s, 10000, stdin) != NULL) {
    char *pch = strtok(s, " ");
    //if (i > 150) break;
    if (0 != window_size && i >= window_size) {
      Transaction<uint32_t> old_transaction = window.front();
      //std::cout << "old trx size " << old_transaction.data()->size() << std::endl;
      removeOldTransaction(old_transaction.data(), &cidListMap, &tableClosed);
      window.pop();
    }
    Transaction<uint32_t> new_transaction = Transaction<uint32_t>(pch, " ", 0);
    if (i % 100 == 0) {
      std::cout << i << " transaction(s) processed" << tableClosed.size() << std::endl;
    }
    //std::cout << "trx size " << new_transaction.data()->size() << std::endl;
    processNewTransaction(new_transaction.data(), &cidListMap, &tableClosed);
    window.push(new_transaction);
    i += 1;

#ifdef DEBUG
    if ((row % 1000 == 0 && row < 10001) || row % 10000 == 0) {
      printf("elapsed time between checkpoint %0.2f ms, ", (clock() - running) / (double)CLOCKS_PER_SEC * 1000);
      running = clock();
      cout << row << " rows processed, idx size/capacity:" << idx.size() << "/" << idx.capacity() << ", # concept:" << fCI2.size() << endl;
    }
#endif
  }
  std::cout << tableClosed.size() << std::endl;
  printf("Stream completed in %0.2f sec, ", (clock() - start) / (double)CLOCKS_PER_SEC);

  {
    std::map<uint32_t, CloStreamCI*>::iterator it = tableClosed.begin();
    for (; it != tableClosed.end(); ++it) {
      CloStreamCI* const ci = it->second;
      delete ci->itemset;
      delete ci->positions_in_lists;
      if(ci->id) delete ci;
    }
  }

#ifdef _WIN32
  //PROCESS_MEMORY_COUNTERS_EX info;
  //GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS *)&info, sizeof(info));
  //std::cout << "WorkingSet " << info.WorkingSetSize / 1024 << "K, PeakWorkingSet " << info.PeakWorkingSetSize / 1024 << "K, PrivateSet " << info.PrivateUsage / 1024 << "K" << endl;
#endif
  return EXIT_SUCCESS;
}
