/* This file defines some basic constants for the MPI machinery.
 * AMD64 compiled for the x32 ABI is special and thus we can't use the
 * standard values for this ABI.  */
#if (__GNUC__ >= 3 && defined(__x86_64__) && defined(__ILP32__)) || (defined(_WIN64) || defined(_MSC_VER) && (defined(__x86_64__) || defined(__x86_64) || defined(_M_X64)))
#define BYTES_PER_MPI_LIMB 8
#else
#define BYTES_PER_MPI_LIMB  (SIZEOF_UNSIGNED_LONG)
#endif
