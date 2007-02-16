#include "AppPrecomp.h"
#include "Camera.h"
#include "Tile.h"
#include "Screen.h"
#include "GameLogic.h"
#include "EntMapCache.h"
#include "MovingEntity.h"
#include "AppUtils.h"

#define C_DEFAULT_LERP 0.3f

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
  m_vecScale = ClampScaleToRange(camSetting.VecScale());
  m_vecScaleTarget = ClampScaleToRange(camSetting.VecTargetScale());
}

CameraSetting Camera::GetCameraSettings()
{
	CameraSetting camSettingOut;

	camSettingOut.VecPos() = m_vecPos;
	camSettingOut.VecTargetPos() = m_vecTargetPos;
	camSettingOut.VecScale() = m_vecScale;
	camSettingOut.VecTargetScale() = m_vecScaleTarget;

	return camSettingOut;
}

CL_Rect Camera::GetScreenRect()
{
	return CL_Rect(0,0, GetScreenX, GetScreenY);
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

void Camera::Reset()
{
	m_bInstantUpdateASAP = false;
	m_vecPos = g_mapDefaultCenterPos;
	m_vecTargetPos = g_mapDefaultCenterPos;
	m_vecScale.x = m_vecScale.y = 1;
	m_vecScaleTarget = m_vecScale;
	m_entTrackID = 0;
	m_moveLerp = C_DEFAULT_LERP;
	m_scaleLerp = C_DEFAULT_LERP;
}

void Camera::SetPos(CL_Vector2 vecPos)
{
	m_vecPos = vecPos;
	m_vecTargetPos = vecPos;
}

void Camera::SetScaleRaw(CL_Vector2 vecScale)
{
	m_vecScale = vecScale;
	m_vecScaleTarget = m_vecScale;
}

void Camera::SetScaleTarget(CL_Vector2 vecScale)
{
	if (!GetActiveMap) return;

	m_vecScaleTarget = ClampScaleToRange(vecScale);
}


CL_Vector2 Camera::ClampScaleToRange(CL_Vector2 vecScale)
{

	const float maxZoom = 40.0f; //larger the number, the closer we can zoom in
	const float minZoom = 0.010f; //smaller the #, the farther we can zoom out

	//make sure current zoom level is within acceptable range
	vecScale.x = min(vecScale.x, maxZoom);
	vecScale.y = min(vecScale.y, maxZoom);

	vecScale.x = max(vecScale.x, minZoom);
	vecScale.y = max(vecScale.y, minZoom);

	return vecScale;
}

void Camera::SetScale(CL_Vector2 vecScale)
{
	if (!GetActiveMap) return;
	CL_Vector2 vecPosBefore;
	if (m_entTrackID == 0)
	{
		vecPosBefore = GetWorldCache->ScreenToWorld(CL_Vector2(GetScreenX, GetScreenY));
	}
	
	m_vecScale = ClampScaleToRange(vecScale);

	m_vecScaleTarget = m_vecScale;
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
		MovingEntity *pEnt = ((MovingEntity*) EntityMgr->GetEntityFromID(m_entTrackID));
		if (pEnt)
		{
			CL_Vector2 vPos = pEnt->GetPos();
			
			//hard coded offset so we don't stare at the feet when zoomed in.  Move later
			vPos.y -= (40*pEnt->GetScale().y);
			SetTargetPosCentered(vPos);
		} else
		{
			LogMsg("Camera can't find Entity %d, disabling camera tracking.", m_entTrackID);
			m_entTrackID = 0;
		}
	} 
}

void Camera::SetTargetPosCentered(CL_Vector2 vecTarget)
{
	m_vecTargetPos = vecTarget;
	m_vecTargetPos.x -= (float(GetScreenX/2))/m_vecScale.x;
	m_vecTargetPos.y -= (float(GetScreenY/2))/m_vecScale.y;

	//m_vecTargetPos.x = RoundNearest(m_vecTargetPos.x,1.0f);
	//m_vecTargetPos.y = RoundNearest(m_vecTargetPos.y,1.0f);

}

CL_Vector2 Camera::GetPosCentered()
{
	return m_vecPos + CL_Vector2((float(GetScreenX/2))/m_vecScale.x, (float(GetScreenX/2))/m_vecScale.y );
}

void Camera::SetPosCentered(CL_Vector2 vecPos)
{
	m_vecPos = vecPos;
	m_vecPos.x -= (float(GetScreenX/2))/m_vecScale.x;
	m_vecPos.y -= (float(GetScreenY/2))/m_vecScale.y;
	m_vecTargetPos = m_vecPos;
}

void Camera::SetTargetPos(CL_Vector2 vecTarget)
{
	m_vecTargetPos = vecTarget;
}
void Camera::InstantUpdate()
{
	UpdateTarget();
	SetScale(m_vecScaleTarget);
	SetPos(m_vecTargetPos);
}

void Camera::Update(float step)
{
	assert(step != 0);
	
	m_vecScale = Lerp(m_vecScale, m_vecScaleTarget, cl_min(m_scaleLerp*step, 1));

	
	UpdateTarget();
  	
	if (m_bInstantUpdateASAP)
	{
		m_bInstantUpdateASAP = false;
		InstantUpdate();
	}
	
	m_vecPos = Lerp(m_vecPos,m_vecTargetPos, cl_min(m_moveLerp*step, 1));

	static const float fuzz = 0.01f;

	if (m_vecScaleTarget.x < m_vecScale.x+fuzz && m_vecScaleTarget.x > m_vecScale.x-fuzz)
	{
		m_vecPos.x = RoundNearest(m_vecPos.x,1.0f);
		m_vecPos.y = RoundNearest(m_vecPos.y,1.0f);
	}

}
