#pragma once
#include "../../../Util/Math/Vector/Vector.h"

class IHandleEntity;
struct Ray_t;
class CGameTrace;
typedef CGameTrace trace_t;

class ICollideable {
public:
  virtual IHandleEntity *GetEntityHandle() = 0;
  virtual const Vector &OBBMins() const = 0;
  virtual const Vector &OBBMaxs() const = 0;
  virtual void WorldSpaceTriggerBounds(Vector *pVecWorldMins,
                                       Vector *pVecWorldMaxs) const = 0;
  virtual bool TestCollision(const Ray_t &ray, unsigned int fContentsMask,
                             trace_t &tr) = 0;
  virtual bool TestHitboxes(const Ray_t &ray, unsigned int fContentsMask,
                            trace_t &tr) = 0;
  virtual int GetCollisionModelIndex() = 0;
  virtual const void *GetCollisionModel() = 0;
  virtual const Vector &GetCollisionOrigin() const = 0;
  virtual const void *GetCollisionAngles() const = 0; // QAngle* actually
  virtual const matrix3x4_t &CollisionToWorldTransform() const = 0;
};
