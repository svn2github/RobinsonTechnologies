
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 25:2:2006   15:05
*/


#ifndef Brain_HEADER_INCLUDED // include guard
#define Brain_HEADER_INCLUDED  // include guard

class MovingEntity;

class Brain
{
public:

	Brain(MovingEntity * pParent);
	virtual ~Brain();
	virtual void Update(float step) = 0;
	virtual void PostUpdate(float step) {};
	virtual const char * GetName()=0;
	virtual Brain * CreateInstance(MovingEntity *pParent)=0;
	int GetSort() const {return m_sort;}
	void SetSort(int sort){m_sort = sort;}
	virtual void OnAdd(){}; //called once when brain is inserted
	virtual void HandleMsg(const string &msg) {return;}
	virtual string HandleAskMsg(const string &msg) {return "";}
	virtual void AddWeightedForce(const CL_Vector2 & force){assert(!"This brain not setup to be a base brain!");};
	virtual unsigned int GetKeys(){return 0;}
	void SetDeleteFlag(bool bFlag) {m_bDeleteFlag = bFlag;}
	bool GetDeleteFlag() {return m_bDeleteFlag;}

protected:

	void RegisterClass();

	int m_sort; //higher # gets run first
	MovingEntity *m_pParent;
	bool m_bDeleteFlag;
};

#endif                  // include guard
