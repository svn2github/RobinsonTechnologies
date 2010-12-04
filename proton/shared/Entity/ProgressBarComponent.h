//  ***************************************************************
//  ProgressBarComponent - Creation date: 05/31/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef ProgressBarComponent_h__
#define ProgressBarComponent_h__

#include "Component.h"
#include "Entity.h"
#include "Renderer/SurfaceAnim.h"

class ProgressBarComponent: public EntityComponent
{
public:
	ProgressBarComponent();
	virtual ~ProgressBarComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

private:

	void OnUpdate(VariantList *pVList);
	void OnRender(VariantList *pVList);
	void OnFileNameChanged(Variant *pDataObject);
	void OnScaleChanged(Variant *pDataObject);
	void OnProgressChanged(Variant *pDataObject);

	float GetVisualProgress();
	void OnVisualProgressChanged(Variant *pDataObject);

	CL_Vec2f *m_pPos2d;
	CL_Vec2f *m_pSize2d;
	CL_Vec2f *m_pScale2d;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	uint32 *m_pBorderColor;
	float *m_pAlpha;
	uint32 *m_pAlignment;
	float *m_pProgress;
	float *m_pProgressOfLastSet;
	float *m_pVisualProgress;
	float m_baseProgress;
	uint32 *m_pInterpolationTimeMS;
	unsigned int m_timeOfLastSet;
	SurfaceAnim *m_pSurf; //only used if we're using a bmp for the progress bar overlay
	string *m_pFileName;

};

#endif // ProgressBarComponent_h__