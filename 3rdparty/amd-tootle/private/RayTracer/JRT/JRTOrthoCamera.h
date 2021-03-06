/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#ifndef JRT_ORTHO_CAMERA_H
#define JRT_ORTHO_CAMERA_H

#include "JRTCamera.h"

class JRTOrthoCamera : public JRTCamera
{
public:
  JRTOrthoCamera(const Vec3f& rPosition, const Vec3f& rDirection,
                 const Vec3f& rUp, float fHorizontalSize);

  void GetRay(float s, float t, Vec3f* origin, Vec3f* delta) const override;

  void SetAspectRatio(float fAspect) override
  {
    m_fAspect = fAspect;
  }

  // returns the position of the given point in camera NDC space
  // if the point is not visible, s and t are each set to -1
  void ProjectPoint(const Vec3f& pt, float* s, float* t) const override;

  const Vec3f& GetPosition() const
  {
    return m_position;
  }
  const Vec3f& GetDirection() const
  {
    return m_direction;
  }
  const Vec3f& GetUp() const
  {
    return m_up;
  }

private:
  float m_fSize;
  float m_fAspect;
  Vec3f m_position;
  Vec3f m_direction;
  Vec3f m_up;
  Vec3f m_right;
};

#endif
