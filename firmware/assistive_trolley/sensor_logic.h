#pragma once
#include <math.h>
#include <stdint.h>
namespace trolley {
enum class Proximity { Clear, Warning, Critical, Fault };
enum class Load { Normal, Overload, Fault };
inline float distanceCm(unsigned long us) { return us ? us * 0.01715f : NAN; }
inline Proximity proximity(float cm, bool irDetected) {
  if (isfinite(cm) && cm >= 2 && cm <= 400 && cm <= 20) return Proximity::Critical;
  if (irDetected) return Proximity::Warning;
  if (!isfinite(cm) || cm < 2 || cm > 400) return Proximity::Fault;
  return cm <= 50 ? Proximity::Warning : Proximity::Clear;
}
inline float weightKg(long raw, long zero, float countsPerKg) {
  if (!isfinite(countsPerKg) || countsPerKg == 0) return NAN;
  return (float(raw) - float(zero)) / countsPerKg;
}
inline Load loadState(float kg, float limit) {
  if (!isfinite(kg) || !isfinite(limit) || limit <= 0 || kg < -0.05f) return Load::Fault;
  return kg >= limit ? Load::Overload : Load::Normal;
}
inline bool elapsed(uint32_t now, uint32_t then, uint32_t interval) { return uint32_t(now-then) >= interval; }
class Filter {
  float v_ = 0; bool ready_ = false;
public:
  void reset() { ready_ = false; }
  float update(float x) { if (!isfinite(x)) { reset(); return NAN; } v_ = ready_ ? 0.25f*x+0.75f*v_ : x; ready_=true; return v_; }
};
inline bool changed(float a,float b) { return isfinite(a)&&isfinite(b)&&fabsf(a-b)>=0.2f; }
}
