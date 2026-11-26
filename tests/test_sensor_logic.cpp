#include <cassert>
#include <cstdio>
#include "../firmware/assistive_trolley/sensor_logic.h"
using namespace trolley;
int main() {
 int n=0;
 #define CHECK(x) do { assert(x); ++n; } while(0)
 CHECK(isnan(distanceCm(0))); CHECK(fabs(distanceCm(1000)-17.15)<0.001);
 CHECK(proximity(20,false)==Proximity::Critical); CHECK(proximity(20.01,false)==Proximity::Warning);
 CHECK(proximity(50,false)==Proximity::Warning); CHECK(proximity(50.01,false)==Proximity::Clear);
 CHECK(proximity(NAN,false)==Proximity::Fault); CHECK(proximity(1,false)==Proximity::Fault); CHECK(proximity(401,false)==Proximity::Fault);
 CHECK(proximity(100,true)==Proximity::Warning); CHECK(proximity(NAN,true)==Proximity::Warning); CHECK(proximity(10,true)==Proximity::Critical);
 CHECK(weightKg(1500,1000,500)==1); CHECK(weightKg(500,1000,-500)==1); CHECK(isnan(weightKg(10,0,0)));
 CHECK(loadState(2,2)==Load::Overload); CHECK(loadState(1.99,2)==Load::Normal); CHECK(loadState(NAN,2)==Load::Fault); CHECK(loadState(-1,2)==Load::Fault);
 Filter f; CHECK(f.update(4)==4); CHECK(f.update(8)==5); CHECK(isnan(f.update(NAN))); CHECK(f.update(2)==2);
 CHECK(changed(0,0.2f)); CHECK(!changed(0,0.19f)); CHECK(!changed(NAN,1)); CHECK(elapsed(5,UINT32_MAX-4,10)); CHECK(!elapsed(5,UINT32_MAX-4,11));
 printf("%d sensor logic assertions passed\n",n);
}
