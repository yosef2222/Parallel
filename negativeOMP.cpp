#include "lodepng.h"
#include <iostream>
#include <omp.h>
#include <chrono>
#include <vector>

using namespace std;

std::vector <unsigned char> negativeFilter(std::vector<unsigned char> image){
  #pragma omp parallel for
   for(int i = 0; i < (int)image.size()-4; i+=4){
      image[i] = (unsigned char) (255 - (int)image[i]) ;
      image[i+1] = (unsigned char) (255 - (int)image[i+1]);
      image[i+2] = (unsigned char) (255 - (int)image[i+2]);
   }
   return image;
}


int main(int argc, char *argv[]) {

  std::vector<unsigned char> image;
  unsigned w, h;
  std::vector<unsigned char> buffer;
  lodepng::State state;
  unsigned error;

  //check if user gave a filename
  if(argc < 3) {
    std::cout << "please provide in and out filename" << std::endl;
    return 0;
  }

  //state.decoder.color_convert = 0;
  //state.decoder.remember_unknown_chunks = 1; //make it reproduce even unknown chunks in the saved image

  lodepng::load_file(buffer, argv[1]);
  error = lodepng::decode(image, w, h, state, buffer);
  if(error) {
    std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    return 0;
  }

  std::vector<unsigned char> negaImage;
  std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();  
  negaImage = negativeFilter(image);
  std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
  
  
  buffer.clear();
  //state.encoder.text_compression = 1;
  
  error = lodepng::encode(buffer, negaImage, w, h, state);
  if(error) {
    std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    return 0;
  }
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
  std::cout << "Program runtime: " << duration.count() << " milliseconds using " << omp_get_num_threads() << " threads" << std::endl;

  string outname = argv[2];
  lodepng::save_file(buffer, outname + "negative.png");
}
