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

void swap(Complex* x,Complex* y)
{//swap two values given pointers
  Complex temp = *x;
  *x = *y;
  *y = temp;
}

void reorder(Complex *data)
{//given an array, reorder it with bit reversal
int reversedIndex = 0;
  //you're not going to have to reorder first or last element
  for(int i=1;i<(N-1);i++)
    {
    reversedIndex = ReverseBits(i);
    if(i!=reversedIndex)//if bits are not a palindrome
      {
      swap(data+i,data+reversedIndex);//swap their values
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
                    
void Transform1D(Complex* h, int N)
{
  // Implement the efficient Danielson-Lanczos DFT here.
  // "h" is an input/output parameter
  // "N" is the size of the array (assume even power of 2)
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
  reorder(ImageData);
  

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
}

int main(int argc, char** argv)
{
  string fn("Tower.txt"); // default file name
  if (argc > 1) fn = string(argv[1]);  // if name specified on cmd line
  // MPI initialization here
  Transform2D(fn.c_str()); // Perform the transform.
}  
  

  
