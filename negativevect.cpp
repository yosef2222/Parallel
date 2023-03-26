#pragma GCC optimize("O3")

#include "lodepng.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <x86intrin.h>
#include <bits/stdc++.h>

using namespace std;

std::vector <unsigned char> negativeFilter(std::vector<unsigned char> image, int width, int height){
    std::vector<unsigned char> negaimage = image;
    const int numPixels = width * height;
    const __m128i negativeMask = _mm_set1_epi8(-1);
    const __m128i alpha = _mm_set_epi32(-1, -1, -1, -1);

     for (int i = 0; i < (int)image.size()-4; i += 4) {
      __m128i pixel = _mm_loadu_si128((__m128i*)&image[i]);
      __m128i negPixel = _mm_sub_epi8(negativeMask, pixel);
      __m128i negPixelAlpha = _mm_add_epi8(negPixel, alpha);
      _mm_storeu_si128((__m128i*)&negaimage[i], negPixelAlpha);
    }
    return negaimage;
  
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
  negaImage = negativeFilter(image,w,h);
  std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
  
  
  buffer.clear();
  //state.encoder.text_compression = 1;
  
  error = lodepng::encode(buffer, negaImage, w, h, state);
  if(error) {
    std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    return 0;
  }
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
  std::cout << "Program runtime: " << duration.count() << " milliseconds" << std::endl;

  string outname = argv[2];
  lodepng::save_file(buffer, outname + "negative.png");
}
