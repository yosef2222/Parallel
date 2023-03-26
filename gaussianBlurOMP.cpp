#include "lodepng.h"
#include <iostream>
#include <omp.h>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <vector>

using namespace std;

std::vector<unsigned char> gaussianBlur(const std::vector<unsigned char>& image, int width, int height, double std_dev) {
    std::vector<unsigned char> blurred_image(image.size());

    // Calculate the kernel size
    int kernel_size = (int)std::ceil(std_dev * 3) * 2 + 1;

    // Create the Gaussian kernel
    std::vector<double> kernel(kernel_size * kernel_size);
    double sum = 0.0;
    #pragma omp parallel for
    for (int y = -kernel_size / 2; y <= kernel_size / 2; y++) {
        
        for (int x = -kernel_size / 2; x <= kernel_size / 2; x++) {
            double val = std::exp(-(x * x + y * y) / (2 * std_dev * std_dev));
            kernel[(y + kernel_size / 2) * kernel_size + (x + kernel_size / 2)] = val;
            sum += val;
        }
    }

    // Normalize the kernel
    #pragma omp parallel for
    for (int i = 0; i < kernel_size * kernel_size; i++) {
        kernel[i] /= sum;
    }

    // Convolve the image with the kernel
    #pragma omp parallel for
    for (int y = 0; y < height; y++) {
        
        for (int x = 0; x < width; x++) {
            double r_sum = 0.0, g_sum = 0.0, b_sum = 0.0, a_sum = 0.0, k_sum = 0.0;
            
            for (int ky = 0; ky < kernel_size; ky++) {
                
                for (int kx = 0; kx < kernel_size; kx++) {
                    int px = x + kx - kernel_size / 2;
                    int py = y + ky - kernel_size / 2;
                    if (px >= 0 && px < width && py >= 0 && py < height) {
                        int idx = (py * width + px) * 4;
                        double k = kernel[ky * kernel_size + kx];
                        r_sum += k * image[idx];
                        g_sum += k * image[idx + 1];
                        b_sum += k * image[idx + 2];
                        a_sum += k * image[idx + 3];
                        k_sum += k;
                    }
                }
            }
            int idx = (y * width + x) * 4;
            blurred_image[idx] = (unsigned char)(r_sum / k_sum);
            blurred_image[idx + 1] = (unsigned char)(g_sum / k_sum);
            blurred_image[idx + 2] = (unsigned char)(b_sum / k_sum);
            blurred_image[idx + 3] = (unsigned char)(a_sum / k_sum);
        }
    }

    return blurred_image;
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
   
  std::vector<unsigned char> gaussianImage;
  std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
  gaussianImage = gaussianBlur(image, w, h, 7.2);
  std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
    
  
  buffer.clear();
  //state.encoder.text_compression = 1;
  error = lodepng::encode(buffer, gaussianImage, w, h, state);
  if(error) {
    std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    return 0;
  }
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
  std::cout << "Program runtime: " << duration.count() << " milliseconds using " << omp_get_num_threads() << " threads" << std::endl;

  string outname = argv[2];
  lodepng::save_file(buffer, outname + "gaussuan.png");
  }
