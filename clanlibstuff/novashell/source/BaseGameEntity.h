 #ifndef BASE_GAME_ENTITY_H
#define BASE_GAME_ENTITY_H
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
//  Name: BaseGameEntity.h
//
//  Desc: Base class to define a common interface for all game
//        entities
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
#include <vector>
#include <string>
#include <iosfwd>
#include "../../SharedLib/include/ClanLib/signals.h"

struct Telegram;
class TileEntity;

class BaseGameEntity
{
public:
  
  enum 
  {
	  C_ENTITY_TYPE_BASE = 0,
	  C_ENTITY_TYPE_MOVING = 1
  };

  virtual ~BaseGameEntity();
    
    virtual void Update(float step){}; 
    virtual void Render(void *pTarget){};

	//use this to grab the next valid ID
    static int   GetNextValidID(){return m_iNextValidID;}
    //this can be used to reset the next ID
    static void  ResetNextValidID(){m_iNextValidID = 1;}
    int          ID()const{return m_ID;}
    
    bool         IsTagged()const{return m_bTag;}
    void         Tag(){m_bTag = true;}
    void         UnTag(){m_bTag = false;}
    
    int          GetType()const{return m_iType;}
    void         SetType(int new_type){m_iType = new_type;}
    virtual void SetName(const std::string &name) {m_EntName = name;}
    const std::string & GetName(){return m_EntName;} 
    bool GetDeleteFlag(){return m_deleteFlag;}
    void SetDeleteFlag(bool bNew);
	virtual BaseGameEntity * CreateClone() {return NULL;}
	void SetTile(TileEntity *pTileEnt) {m_pTile = pTileEnt;}
	TileEntity * GetTile() {return m_pTile;}
	virtual void HandleMessageString(const std::string &msg){};

	CL_Signal_v1<int> sig_delete; //anyone can attach to this to know when it dies.  Sends its ID #.

private:
  
  //each entity has a unique ID
  int         m_ID;
  bool m_deleteFlag; //if true, this entities will get deleted at the end of the update

 
  //this is a generic flag. 
  bool        m_bTag;

  //this is the next valid ID. Each time a BaseGameEntity is instantiated
  //this value is updated
  static int  m_iNextValidID;

  //this must be called within each constructor to make sure the ID is set
  //correctly. It verifies that the value passed to the method is greater
  //or equal to the next valid ID, before setting the ID and incrementing
  //the next valid ID
  void SetID(int val);

  std::string m_EntName; 
  
protected:
	//every entity has a type associated with it (health, troll, ammo etc)
	int         m_iType;
  BaseGameEntity(int ID);
  TileEntity *m_pTile; //NULL unless we're attached to a tile

};



      
#endif




