#pragma once
#include <algorithm>
namespace fm {
// Smooth delivery times, with a neutral band to avoid oscillating on small changes.
class AdaptiveQuality {
public:
 void reset(){average_=150;level_=1;}
 void observe(double milliseconds){
  average_=average_*0.75+std::clamp(milliseconds,1.0,10000.0)*0.25;
  if(average_>600)level_=0;
  else if(average_>250)level_=1;
  else if(average_<170)level_=2;
 }
 int level()const{return level_;}
 double averageMillis()const{return average_;}
private:
 double average_=150;
 int level_=1;
};
}
