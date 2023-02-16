#include <algorithm>

#include "parallel.h"

using namespace parlay;

inline uint64_t hashf(uint64_t u){
	u = (u * 54059) ^ 76963;
	return u;
}

template <class T>
void scan_down(T *A, T *B, T *lsum, size_t n, T offset){
	if(n == 1){
		B[0] = offset + A[0];
		return;
	}
	size_t m = n/2;
	auto f1 = [&]() { scan_down(A, B, lsum, m, offset); };
	auto f2 = [&]() { scan_down(A+m, B+m, lsum+m, n-m, offset + lsum[m-1]); };

	par_do(f1, f2);
}

template <class T>
T scan_up(T *A, T *lsum, size_t n){
	if(n == 1)
		return A[0];
	size_t m = n/2;
	T l, r;
	auto f1 = [&]() { l = scan_up(A, lsum, m); };
	auto f2 = [&]() { r = scan_up(A + m, lsum + m, n-m); };

	par_do(f1, f2);
	lsum[m-1] = l;
	return l + r;
}

template <class T>
void scan(T *A, T *B, size_t n){
	T* lsum = (T*)malloc((n-1) * sizeof(T));
	scan_up(A, lsum, n);
	scan_down(A, B, lsum, n, T(0));
	free(lsum);
}

template <class T>
void filter(T *A, T *B, size_t n, T a, size_t& p1, size_t &p2){
    
    auto f1 = [&a](T x){ return x < a ? 1 : 0; };
    auto f2 = [&a](T x){ return x > a ? 1 : 0; };
    //auto f3 = [&a](T x){ return x == a ? 1 : 0; };
    
    T* flag1 = (T*)malloc(n * sizeof(T));
    T* flag2 = (T*)malloc(n * sizeof(T));
    //T* flag3 = (T*)malloc(n * sizeof(T));

    T* ps1 = (T*)malloc(n * sizeof(T));
    T* ps2 = (T*)malloc(n * sizeof(T));
    //T* ps3 = (T*)malloc(n * sizeof(T));

    parallel_for(0, n, [&](size_t i) {
        flag1[i] = f1(A[i]);
        flag2[i] = f2(A[i]);
        //flag3[i] = f3(A[i]);
    });
    
    auto s1 = [&]() { scan(flag1, ps1, n); };
    auto s2 = [&]() { scan(flag2, ps2, n); };
    //auto s3 = [&]() { scan(flag3, ps3, n); };
    par_do(s1, s2);
    //scan(flag1, ps1, n);
    //scan(flag2, ps2, n);
    //scan(flag3, ps3, n);
    T ps3 = n - (size_t)ps1[n-1] - (size_t)ps2[n-1];
    
    parallel_for(0, n, [&](size_t i){
        if(f1(A[i]))
            B[ps1[i]-1] = A[i];
        else if(f2(A[i]))
            B[ps1[n-1] + ps3 + ps2[i] - 1] = A[i];
        else{
            B[ps1[n-1] + ps3 - 1] = A[i];
        }
    });
    
    parallel_for(0, n, [&](size_t i){
        A[i] = B[i];
    });
    p1 = (size_t)ps1[n-1], p2 = ps3;
}

template <class T>
void parallel_quicksort(T *A, T *B, size_t n, uint64_t a){
    size_t p1, p2;
    if(n <= 1)
        return;
    size_t pivotIndex = size_t((hashf(a++)) % n);
    filter(B, A, n, B[pivotIndex], p1, p2);
    //parallel_quicksort(A, B, p1, a);
    //parallel_quicksort(A + p1 + p2, B + p1 + p2, n - p1 - p2, a);
    
    auto f1 = [&]() { parallel_quicksort(A, B, p1, a); };
    auto f2 = [&]() { parallel_quicksort(A + p1 + p2, B + p1 + p2, n - p1 - p2, a);};
    par_do(f1, f2);
}

template <class T>
void quicksort(T *A, size_t n) {
	T* B = (T*)malloc(n * sizeof(T));
    parallel_for(0, n, [&](size_t i){
        B[i] = A[i];
    });
    size_t pivotIndex;
    if(n <= 1)
        return;
    pivotIndex = hashf(rand())%n;

    parallel_quicksort(A, B, n, A[pivotIndex]);
    std::cout << "Sorted" << std::endl;
	//for(size_t i = 0; i < n; i++)
        //std::cout << "A[" << i << "]: " << A[i] << "\tB[" << i << "]: " << B[i] << std::endl;
  //std::sort(A, A + n);
}
