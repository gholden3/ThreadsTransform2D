// Threaded two-dimensional Discrete FFT transform
// Gina Holden
// ECE8893 Project 2


#include <iostream>
#include <string>
#include <math.h>

#include "Complex.h"
#include "InputImage.h"

// You will likely need global variables indicating how
// many threads there are, and a Complex* that points to the
// 2d image being transformed.
Complex* ImageData;
int ImageWidth;
int ImageHeight;
int nThreads = 16;
int N = 1024;
Complex* weights = new Complex[N/2];
pthread_mutex_t exitMutex;
pthread_cond_t exitCond;

using namespace std;

// Function to reverse bits in an unsigned integer
// This assumes there is a global variable N that is the
// number of points in the 1D transform.
unsigned ReverseBits(unsigned v)
{ //  Provided to students
  unsigned n = N; // Size of array (which is even 2 power k value)
  unsigned r = 0; // Return value
   
  for (--n; n > 0; n >>= 1)
    {
      r <<= 1;        // Shift return value
      r |= (v & 0x1); // Merge in next bit
      v >>= 1;        // Shift reversal value
    }
  return r;
}

void calcWeights(){
  for(int i=0;i<N/2;i++)
  {
  weights[i] = Complex(cos(2*M_PI*i/N),-sin(2*M_PI*i/N));
  }
}

void swap(Complex* x,Complex* y)
{//swap two values given pointers
  Complex temp = *x;
  *x = *y;
  *y = temp;
}

void reorder()
{//given an array, reorder it with bit reversal
  //do it row by row
  Complex * temp = new Complex[N]; //temp array to hold working values for this row
  for(int r=0;r<N;r++){//for each row in data
    int reversedIndex = 0;
    
    //you're not going to have to reorder first or last element
    for(int i=0;i<N;i++)//for each point in the row
      {
      reversedIndex = ReverseBits(i);
      if(reversedIndex!=i){
        temp[i] = ImageData[r*N+reversedIndex];
        }
      }
   //now copy temp back into data
   for(int j=0;j<N;j++){
     ImageData[r*N+j] = temp[j];
     }
   }
}

// GRAD Students implement the following 2 functions.
// Undergrads can use the built-in barriers in pthreads.

// Call MyBarrier_Init once in main
void MyBarrier_Init()// you will likely need some parameters)
{
}

// Each thread calls MyBarrier after completing the row-wise DFT
void MyBarrier() // Again likely need parameters
{
}
                    
void Transform1D(Complex* h)
{
  // Implement the efficient Danielson-Lanczos DFT here.
  // "h" is an input/output parameter
  // "N" is the size of the array (assume even power of 2)
  Complex temp;
  int points = 2; //loop over number of points: ie  2,4,8,16...64
  for(points=2;points<=N;points=points*2){
    for(int i=0;i<N;i=i+points){ //for each group
      for(int j=0;j<(points/2);j++){//for each point in the group(first half)
        int offset = points/2;
        temp = h[i+j];
        h[i+j] = h[i+j] + weights[j*N/points] * h[i+j+offset];
        h[i+j+offset] = temp - weights[j*N/points] * h[i+j+offset];
      }
    }
  }
}

void* Transform2DThread(void* v)
{ // This is the thread startign point.  "v" is the thread number
  // Calculate 1d DFT for assigned rows
  // wait for all to complete
  // Calculate 1d DFT for assigned columns
  // Decrement active count and signal main if all complete
  return 0;
}

void Transform2D(const char* inputFN) 
{ // Do the 2D transform here.
  InputImage image(inputFN);  // Create the helper object for reading the image
  // Create the global pointer to the image array data
  ImageData = image.GetImageData();
  ImageWidth = image.GetWidth();
  ImageHeight = image.GetHeight();
  //just do the 1D on the whole image without threads first
  //reorder the entire matrix
  reorder();
  calcWeights();
  int i = 0;
  Complex * myPtr = 0;
  for(i=0;i<ImageHeight;i++){
    myPtr = ImageData+(i*ImageWidth);
    Transform1D(myPtr);
  }
/*
  pthread_mutex_init(&exitMutex,0);
  pthread_cond_init(&exitCond,0);
  pthread_mutex_lock(&exitMutex);
  // Create 16 threads
  for (int i=0;i<nThreads;++i)
    {
    pthread_t pt;
    pthread_create(&pt,0,Transform2DThread,(void*)i);
    }
  // Wait for all threads complete
  pthread_cont_wait(&exitCond,&exitMutex);
  // Write the transformed data*/
  image.SaveImageData("MyAfter1D.txt",ImageData,ImageWidth,ImageHeight);
}

int main(int argc, char** argv)
{
  string fn("Tower.txt"); // default file name
  if (argc > 1) fn = string(argv[1]);  // if name specified on cmd line
  // MPI initialization here
  Transform2D(fn.c_str()); // Perform the transform.
}  
  

  
