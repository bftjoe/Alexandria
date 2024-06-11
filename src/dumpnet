// utility to dump net reprensentation to disk in the form of header file, add to src/nnue.h

#include <fstream>

inline static void dumpNet(){
  using namespace std;
  
  ofstream outf{ "FTWeights.h" };
  outf << "constexpr int16_t FTWeights[NUM_INPUTS * L1_SIZE] = {";
  for( int i = 0; i < NUM_INPUTS * L1_SIZE; i++){
    outf << net.FTWeights[i];
    if (i < NUM_INPUTS * L1_SIZE - 1)
      outf << ",";
  }
  outf << "};\n";
  
  ofstream outf2{ "net.h" };
  outf << "constexpr int16_t FTBiases[L1_SIZE] = {";
  for( int i = 0; i < L1_SIZE; i++){
    outf2 << net.FTBiases[i];
    if (i < L1_SIZE - 1)
      outf2 << ",";
  }
  outf2 << "};\n";
  
  

  outf2 << "constexpr int16_t L1Biases[OUTPUT_BUCKETS] = {";
  for( int i = 0; i < OUTPUT_BUCKETS; i++){
    outf2 << net.L1Biases[i];
    if (i < OUTPUT_BUCKETS - 1)
      outf2 << ",";
  }
  outf2 << "};\n"; 
  
  ofstream outf{ "L1Weights.h" };
  outf << "constexpr int16_t L1Weights[L1_SIZE * 2 * OUTPUT_BUCKETS] = {";
  for( int i = 0; i < L1_SIZE * 2 * OUTPUT_BUCKETS; i++){
    outf << net.L1Weights[i];
    if (i < L1_SIZE * 2 * OUTPUT_BUCKETS - 1)
      outf << ",";
  }
  outf << "};\n";
}
