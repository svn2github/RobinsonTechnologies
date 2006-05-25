#include "AppPrecomp.h"
#include "Camera.h"
#include "Tile.h"
#include "Screen.h"
#include "GameLogic.h"
#include "EntWorldCache.h"
#include "MovingEntity.h"

Camera::Camera()
{
	Reset();
}

Camera::~Camera()
{
}

void Camera::SetCameraSettings(CameraSetting &camSetting)
{
  m_vecPos = camSetting.VecPos();
  m_vecTargetPos = camSetting.VecTargetPos();
  m_vecScale = camSetting.VecScale();
}

CL_Rectf Camera::GetViewRectWorld()
{
	CL_Rectf r;
	CL_Vector2 v;
	v = GetWorldCache->ScreenToWorld(CL_Vector2(GetScreenX, GetScreenY));
	r.right = v.x;
	r.bottom = v.y;

	r.left = m_vecPos.x;
	r.top = m_vecPos.y;
	return r;
}

void Camera::GetCameraSettings(CameraSetting &camSettingOut)
{
	camSettingOut.VecPos() = m_vecPos;
	camSettingOut.VecTargetPos() = m_vecTargetPos;
	camSettingOut.VecScale() = m_vecScale;

}
void Camera::Reset()
{
	m_bInstantUpdateASAP = false;
	m_vecPos = g_worldDefaultCenterPos;
	m_vecTargetPos = g_worldDefaultCenterPos;
	m_vecScale.x = m_vecScale.y = 1;
	m_entTrackID = 0;
}

void Camera::SetPos(CL_Vector2 vecPos)
{
	m_vecPos = vecPos;
	m_vecTargetPos = vecPos;
}

void Camera::SetScaleRaw(CL_Vector2 vecScale)
{
	m_vecScale = vecScale;
}
void Camera::SetScale(CL_Vector2 vecScale)
{
	if (!GetWorld) return;
	
	const float maxZoom = 30.0f; //larger the number, the closer we can zoom in
	const float minZoom = 0.010f; //smaller the #, the farther we can zoom out
	CL_Vector2 vecPosBefore;

	if (m_entTrackID == 0)
	{
		vecPosBefore = GetWorldCache->ScreenToWorld(CL_Vector2(GetScreenX, GetScreenY));
	}
	
	m_vecScale = vecScale;

	//make sure current zoom level is within acceptable range
	m_vecScale.x = min(m_vecScale.x, maxZoom);
	m_vecScale.y = min(m_vecScale.y, maxZoom);

	m_vecScale.x = max(m_vecScale.x, minZoom);
	m_vecScale.y = max(m_vecScale.y, minZoom);

	if (m_entTrackID == 0)
	{
		m_vecTargetPos +=  (vecPosBefore-GetWorldCache->ScreenToWorld(CL_Vector2(GetScreenX, GetScreenY)))/2;
	}

}

void Camera::SetEntTracking(int entID)
{
	m_entTrackID = entID;
}


void Camera::UpdateTarget()
{
	if (m_entTrackID != 0)
	{
		CL_Vector2 vPos = ((MovingEntity*) EntityMgr->GetEntityFromID(m_entTrackID))->GetPos();
		SetTargetPosCentered(vPos);
	} 
}

void Camera::SetTargetPosCentered(CL_Vector2 vecTarget)
{
	m_vecTargetPos = vecTarget;
	m_vecTargetPos.x -= (GetScreenX/2)/m_vecScale.x;
	m_vecTargetPos.y -= (GetScreenY/2)/m_vecScale.y;
}

CL_Vector2 Camera::GetPosCentered()
{
	return m_vecPos + CL_Vector2((GetScreenX/2)/m_vecScale.x, (GetScreenX/2)/m_vecScale.y );
}

void Camera::SetPosCentered(CL_Vector2 vecPos)
{
	m_vecPos = vecPos;
	m_vecPos.x -= (GetScreenX/2)/m_vecScale.x;
	m_vecPos.y -= (GetScreenY/2)/m_vecScale.y;
}

void Camera::SetTargetPos(CL_Vector2 vecTarget)
{
	m_vecTargetPos = vecTarget;
}
void Camera::InstantUpdate()
{
	UpdateTarget();
	SetPos(m_vecTargetPos);
}

void Camera::Update(float step)
{
	UpdateTarget();
  	
	if (m_bInstantUpdateASAP)
	{
		m_bInstantUpdateASAP = false;
		InstantUpdate();
	}
	
	SetPos((m_vecPos + m_vecTargetPos)/2);
}
