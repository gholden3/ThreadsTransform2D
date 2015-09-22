// Threaded two-dimensional Discrete FFT transform
// Gina Holden
// ECE8893 Project 2


#include <iostream>
#include <string>
#include <math.h>
#include <pthread.h>
#include <pthread.h>
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
pthread_mutex_t startCountMutex;
pthread_mutex_t exitMutex;
pthread_cond_t exitCond;
pthread_barrier_t barr;
pthread_barrier_t barrT;
int startCount;

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
      else{
        temp[i] = ImageData[r*N+i];
        }
      }
   //now copy temp back into data
   for(int j=0;j<N;j++){
     ImageData[r*N+j] = temp[j];
     }
   }
}

void transposeMatrix(){
  Complex * temp = new Complex[ImageWidth*ImageHeight];
  //copy ImageData into temp arr
  for(int i=0;i<ImageHeight;i++){
    for(int j =0;j<ImageWidth;j++){
      temp[i*ImageWidth+j] = ImageData[i*ImageWidth+j];
    }
  }
  //loop over temp array to put into ImageData
  for(int i=0;i<ImageHeight;i++){
    for(int j=0;j<ImageWidth;j++){
      ImageData[i*ImageWidth+j] = temp[j*ImageWidth+i];
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
  // no mutexes needed for row transforms. 
  // no threads will be working on the same rows (hopefully)
  Complex temp;
  int points = 2; //loop over number of points: ie  2,4,8,16...64
  for(points=2;points<=N;points=points*2){
    for(int i=0;i<N;i=i+points){ //for each group
      for(int j=0;j<(points/2);j++)
        {//for each point in the group(first half)
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
  unsigned long myID = (unsigned long)v; // my thread number
  int rowsPerThread = ImageHeight / nThreads;
  int startingRow = myID * rowsPerThread;\
  int endingRow = startingRow + rowsPerThread;
  // Calculate 1d DFT for assigned rows
  for(int sr = startingRow;sr<endingRow;sr++)
    {
    Complex* rowPtr = ImageData + ImageWidth*sr;
    Transform1D(rowPtr);
    }
  // wait for all to complete
   pthread_barrier_wait(&barr);
  //wait for transpose of matrix
  pthread_barrier_wait(&barrT);
  //now im ready to do 1D dft on cols
  for(int sr = startingRow;sr<endingRow;sr++)
    {
    Complex* rowPtr = ImageData + ImageWidth*sr;
    Transform1D(rowPtr);
    }
  pthread_mutex_lock(&startCountMutex);
  //decrement active count and signify main if complete
  startCount--;
  if (startCount==0)
    { //last to exit. notify main
    pthread_mutex_unlock(&startCountMutex);
    pthread_mutex_lock(&exitMutex);
    pthread_cond_signal(&exitCond);
    pthread_mutex_unlock(&exitMutex);
    }
  else
    {
    pthread_mutex_unlock(&startCountMutex);
    }
  return 0;
}

void Transform2D(const char* inputFN) 
{ // Do the 2D transform here.
  InputImage image(inputFN);  // Create the helper object for reading the image
  // Create the global pointer to the image array data
  ImageData = image.GetImageData();
  ImageWidth = image.GetWidth();
  ImageHeight = image.GetHeight();
  //reorder the entire matrix
  reorder();
  calcWeights();
  //barrier initialization
  pthread_barrier_init(&barr, NULL, nThreads+1);
  pthread_barrier_init(&barrT,NULL,nThreads+1);
  pthread_t pt;
  for (int i=0;i<nThreads;i++)
    {
    pthread_create(&pt,0,Transform2DThread,(void*)i);
    }
  pthread_barrier_wait(&barr);
  image.SaveImageData("MyAfter1D.txt",ImageData,ImageWidth,ImageHeight);
  transposeMatrix();
  reorder();
  calcWeights();
  //all threads have to wait for me to transpose and reorder the matrix
  pthread_barrier_wait(&barrT);
  //restart the threads
  //wait for all to finish column transforms
  pthread_cond_wait(&exitCond,&exitMutex);
  transposeMatrix();
  image.SaveImageData("Tower-DFT2D.txt",ImageData,ImageWidth,ImageHeight);
}

int main(int argc, char** argv)
{ 
  string fn("Tower.txt"); // default file name
  if (argc > 1) fn = string(argv[1]);  // if name specified on cmd line
  pthread_mutex_init(&exitMutex,0);
  pthread_mutex_init(&startCountMutex,0);
  pthread_cond_init(&exitCond,0);
  //holds exit mutex
  pthread_mutex_lock(&exitMutex);
  startCount = nThreads;
  Transform2D(fn.c_str()); //get things start  the transform
}  
  

  
