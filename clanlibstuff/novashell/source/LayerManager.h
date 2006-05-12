
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 2:2:2006   15:10
*/


#ifndef LayerManager_HEADER_INCLUDED // include guard
#define LayerManager_HEADER_INCLUDED  // include guard

#define C_LAYER_FILENAME "layer.dat"
const int C_LAYER_VERSION = 0;
//some help to name these for generation functions, not really hardcoded though
enum
{
	C_LAYER_BACKGROUND1 = 0,
	C_LAYER_BACKGROUND2,
	C_LAYER_BACKGROUND3,
	C_LAYER_MAIN,
	C_LAYER_DETAIL1,
	C_LAYER_DETAIL2,
	C_LAYER_ENTITY,
	C_LAYER_OVERLAY1,
	C_LAYER_OVERLAY2,
	C_LAYER_HIDDEN_DATA,

	//add more above here
	C_LAYER_DEFAULT_COUNT
};

class Layer
{
public:
	Layer();

	byte IsDisplayed(){return m_byteArray[e_byteIsDisplayed];}
	void SetIsDisplayed(byte isDisplayed){m_byteArray[e_byteIsDisplayed] = isDisplayed;}

	byte IsEditActive(){return m_byteArray[e_byteEditActive];}
	void SetIsEditActive(byte isEditActive){m_byteArray[e_byteEditActive] = isEditActive;}

	byte GetShowInEditorOnly() {return m_byteArray[e_byteShowInEditorOnly];};
	void SetShowInEditorOnly(byte showInEditorOnly) {m_byteArray[e_byteShowInEditorOnly] = showInEditorOnly;}

	byte GetUseInThumbnail() {return m_byteArray[e_byteUseInThumbnail];};
	void SetUseInThumbnail(byte useInThumbnail) {m_byteArray[e_byteUseInThumbnail] = useInThumbnail;}

	byte GetUseParallaxInThumbnail() {return m_byteArray[e_byteUseParallaxInThumbnail];};
	void SetUseParallaxInThumbnail(byte useParallaxInThumbnail) {m_byteArray[e_byteUseParallaxInThumbnail] = useParallaxInThumbnail;}

	const string & GetName(){return m_stName;}
	void SetName(const string st){m_stName = st;}
	
	void SetSort(int sort) { m_intArray[e_intSort] = sort;}
	int GetSort() {return m_intArray[e_intSort];}
	CL_Vector2 GetScrollMod() { return CL_Vector2(m_floatArray[e_floatScrollModX], m_floatArray[e_floatScrollModY]);}
	void SetScrollMod(const CL_Vector2 &scrollMod);
	void Serialize(CL_FileHelper &helper); //handles loading and saving to a stream
	
private:

	
	enum
	{
		e_intSort = 0,
		
		//add new vars above here
		e_intCount
	};
	enum
	{
		e_byteIsDisplayed = 0,
		e_byteEditActive,
		e_byteShowInEditorOnly,
		e_byteUseInThumbnail,
		e_byteUseParallaxInThumbnail,

		//add new vars above here
		e_byteCount
	};

	enum
	{
		e_floatScrollModX = 0,
		e_floatScrollModY,

		//add new vars above here
		e_floatCount
	};

	enum
	{
		e_uintPlaceHolder = 0, 

		//add new vars above here
		e_uintCount
	};

	cl_uint8 m_byteArray[e_byteCount];
	cl_int32 m_intArray[e_intCount];
	cl_uint32 m_uintArray[e_uintCount];
	float m_floatArray[e_floatCount];

	string m_stName;
	
};

typedef vector<Layer> layer_vector;

class LayerManager
{
public:

    LayerManager();
    virtual ~LayerManager();
	unsigned int GetLayerCount() {return m_layerVec.size();}
	Layer & GetLayerInfo(unsigned int layerID) {return m_layerVec[layerID];}
	vector<unsigned int> & GetDrawList() {return m_drawList;}
	vector<unsigned int> & GetEditActiveList() {return m_editActiveList;}
	void BuildLists();
	void PopulateIDVectorWithAllLayers(vector<unsigned int> &layerIDVecOut);
	void Remove(int layerID);
	void Add(Layer layer);
	void Save(const string &fileName);
	void Load(const string &filename);

protected:

	void BuildDefaultLayers();

	layer_vector m_layerVec;
	vector<unsigned int> m_drawList, m_editActiveList;
};


#endif                  // include guard
