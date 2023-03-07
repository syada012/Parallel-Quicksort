#include "quicksort.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>

#include "get_time.h"

using Type = long long;

inline uint64_t hash64(uint64_t u) {
  uint64_t v = u * 3935559000370003845ul + 2691343689449507681ul;
  v ^= v >> 21;
  v ^= v << 37;
  v ^= v >> 4;
  v *= 4768777513237032717ul;
  v ^= v << 20;
  v ^= v >> 41;
  v ^= v << 5;
  return v;
}

int main(int argc, char* argv[]) {
  size_t n = 1e8;
  int num_rounds = 3;
  int random_seed = 1;
  if (argc >= 2) {
    n = atoll(argv[1]);
  }
  if (argc >= 3) {
    num_rounds = atoi(argv[2]);
  }
  if (argc >= 4) {
    random_seed = atoi(argv[3]);
  }

  Type* A = (Type*)malloc(n * sizeof(Type));
  Type* B = (Type*)malloc(n * sizeof(Type));

  double total_time = 0;
  for(int xyz = 0, abc = 0; xyz< 5; xyz++){
  for (int i = 0; i <= num_rounds; i++) {
    // Generate random arrays
    if(xyz == 0){
    	parallel_for(0, n, [&](size_t j) {
		A[j] = B[j] = hash64(j * random_seed);
	});
	if((abc++) == 0)
		std::cout << "All distinct, uniformly random: " << std::endl;
    }
    if(xyz == 1){
    	parallel_for(0, n, [&](size_t j) {
		A[j] = B[j] = (hash64(j * random_seed)%(n/10));
	});
	if((abc++) == 4)
		std::cout << "Light duplicates, uniformly random: " << std::endl;
    }
    if(xyz == 2){
	parallel_for(0, n, [&](size_t j) {
		if(j < n/4)
			A[j] = B[j] = (hash64(j * random_seed)%(n/10));
		else if(j < (2*(n/4)))
			A[j] = B[j] = (hash64(j * random_seed)%(n/5));
		else
			A[j] = B[j] = (hash64(j * random_seed)%(n/2));
	});
	if((abc++) == 8)
		std::cout << "Light duplicates, skewed distribution: " << std::endl;
    }
    if(xyz == 3){
    	parallel_for(0, n, [&](size_t j) {
		A[j] = B[j] = (hash64(j * random_seed)%10000);
	});
	if((abc++) == 12)
		std::cout << "Heavy duplicates, uniformly random: " << std::endl;
    }
    if(xyz == 4){
    	parallel_for(0, n, [&](size_t j) {
		if(j < (n/4))
			A[j] = B[j] = ((j * random_seed)%10000);
		else if(j < (2*(n/4)))
			A[j] = B[j] = (hash64(j * random_seed)%(n/90000));
		else
			A[j] = B[j] = (hash64(j * random_seed)%(1000));
	});
	if((abc++) == 16)
		std::cout << "Heavy duplicates, skewed distribution: " << std::endl;
    }

    parlay::timer t;
    quicksort(A, n);
    t.stop();

    std::sort(B, B + n);
    parallel_for(0, n, [&](size_t j) {
      if (A[j] != B[j]) {
        std::cout << "The output is not sorted\n";
        exit(0);
      }
    });

    if (i == 0) {
      std::cout << "Warmup round running time: " << t.total_time() << std::endl;
    } else {
      std::cout << "Round " << i << " running time: " << t.total_time()
                << std::endl;
      total_time += t.total_time();
    }
  }
  std::cout << "Average running time: " << total_time / num_rounds << std::endl;
  total_time = 0;
  }

  free(A);
  return 0;
}
