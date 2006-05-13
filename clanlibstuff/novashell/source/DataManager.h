// ***************************************************************
//  DataManager - date created: 04/20/2006
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com) 
//  Copyright (C) 2006 Robinson Technologies - All Rights Reserved

#ifndef DataManager_HEADER_INCLUDED // include guard
#define DataManager_HEADER_INCLUDED  // include guard

using namespace std;

//entites each hold one of these, it needs to be able to handle copying and stuff

class DataObject
{
public:

	string Get(); //returns value as string, converts # if required
	void Set(const string &keyName, const string &value);
	void SetNum(const string &keyName, float value);
	float GetNum();

	void Serialize(CL_FileHelper &helper);

	DataObject()
	{
		m_dataType = 0;
		m_num = 0;
		
	}
	
	cl_uint8 m_dataType;

	string m_key;
	string m_value;
	float m_num;
	
};

typedef std::map<string, DataObject> dataList;

class DataManager
{
public:

	enum
	{
		E_STRING,
		E_NUM
	};

	string Get(const string &keyName);
	
	//returns true if it also created it
	bool Set(const string &keyName, const string &value);
	bool Exists(const string &keyName); //returns true if key exists
	void Delete(const string &keyName);
	void Clear();
	bool SetIfNull(const string &keyName, const string &value);
	float GetNum(const string &keyName);
	bool SetNum(const string &keyName, float num);
	float ModNum(const string &keyName, float mod);
	
	void Serialize(CL_FileHelper &helper);

	dataList * GetList() {return &m_data;} //so we can work with the raw container outside the class

	DataManager();
    ~DataManager();

private:

	DataObject * FindDataByKey(const string &keyName);
	dataList m_data;

};

#endif                  // include guard