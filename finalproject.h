#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <chrono>
#include "omp.h"
#include <limits.h>
#include "math.h"

__attribute__((noinline))
void magnetForce(
  const int numMag,
  const int numObj,
  const double tesla,
  double const* mag, 
  double* obj,
  const double* polarityValues,     //1 if positive, -1 if negative
  const int objectPolarity,         //1 if positive, 0 if negative
  bool offloadFlag
) 
{  
   double closest = INT_MAX;
   double sum = 0;
   int closObj;
   int closMag;
   double vec[3];
   
   //offloads all the input arrays and needed variables with the conditional boolean attribute
   #pragma offload target(mic:1) if(offloadFlag) in(numMag) in(numObj) in(tesla) \
      in(mag:length(numMag*3)) in(obj:length(numObj*3)) in(polarityValues:length(numMag)) \
      in(objectPolarity) in(closest) inout(sum) inout(closObj) inout(closMag) inout(vec)
   {  
   #pragma omp parallel for reduction (+: sum) 
   for (int i=0; i < numMag; i++) {
      //determines if value added to sum represents attraction or repulsion
      double magFactor = objectPolarity ? polarityValues[i] : -polarityValues[i];
      for (int j=0; j < numObj; j++) {
         
         double dist = sqrt(pow(mag[i*3+0] - obj[j*3+0],2) + pow(mag[i*3+1] - obj[j*3+1],2) 
            + pow(mag[i*3+2] - obj[j*3+2],2));
            
         #pragma omp critical 
         {
            //determines the closest points between the magnet and the object
            if (dist < closest) {
               closest = dist;
               closMag = i;
               closObj = j;
            }
         }
         //value of magnetic influence exponentially decreases with distance,
         sum += tesla / (magFactor * pow(dist, 2));
      }
   }
   }
   
   //compute average magnetic influence per vertex
   double avg = sum * (1.0 / numObj);
   
   //vector between the two closest points
   vec[0] = obj[closObj*3+0] - mag[closMag*3+0];
   vec[1] = obj[closObj*3+1] - mag[closMag*3+1];
   vec[2] = obj[closObj*3+2] - mag[closMag*3+2];
   
   double norm = sqrt(pow(vec[0],2) + pow(vec[1],2) + pow(vec[2],2));
   
   //object can only be attracted or repelled a distance less than or equal
   //to the components of the closest points vector
   vec[0] = fabs((vec[0] / norm) * avg) >= fabs(vec[0]) ? vec[0] : (vec[0] / norm) * avg;
   vec[1] = fabs((vec[1] / norm) * avg) >= fabs(vec[1]) ? vec[1] : (vec[1] / norm) * avg;
   vec[2] = fabs((vec[2] / norm) * avg) >= fabs(vec[2]) ? vec[2] : (vec[2] / norm) * avg;
   
   //updates positions of vertices
   if (tesla != 0) {
      #pragma simd
      for (int j = 0; j < numObj; j++) {
         obj[j*3+1] = obj[j*3+1] + vec[1];
         obj[j*3+2] = obj[j*3+2] + vec[2];
         obj[j*3+0] = obj[j*3+0] + vec[0];                  
      }
   }
}   

