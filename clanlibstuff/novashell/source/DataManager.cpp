#include "AppPrecomp.h"
#include "DataManager.h"

string DataObject::Get()
{
	if (m_dataType == DataManager::E_STRING) return m_value;

	//must be a number
	assert(m_dataType == DataManager::E_NUM);
	return CL_String::from_float(m_num);
}

void DataObject::Set(const string &keyName, const string &value)
{
	m_key = keyName;
	m_value = value;
	m_dataType = DataManager::E_STRING;
}

void DataObject::SetNum(const string &keyName, float value)
{
	m_key = keyName;
	m_num = value;
	m_dataType = DataManager::E_NUM;
}



float DataObject::GetNum()
{
	if (m_dataType == DataManager::E_NUM) return m_num;

	return CL_String::to_float(m_value);
}



void DataObject::Serialize(CL_FileHelper &helper)
{
	//loads or saves to disk

	helper.process(m_dataType);
	helper.process(m_key);
	helper.process(m_value);
}

DataManager::DataManager()
{
}

DataManager::~DataManager()
{
}

void DataManager::Clear()
{
	m_data.clear();
}

//modify the number by a num, creates the key if it doesn't exist
float DataManager::ModNum(const string &keyName, float value)
{
	
	DataObject *pData = FindDataByKey(keyName);

	if (!pData)
	{
		//we need to create it too it looks like
		DataObject d;
		d.SetNum(keyName, value);
		m_data[keyName] = d;
		return value;
	}

	pData->m_num += value;
	pData->m_value.clear();
	pData->m_dataType = DataManager::E_NUM;
  return pData->m_num;
}

DataObject * DataManager::FindDataByKey(const string &keyName)
{
	
	dataList::iterator itor = m_data.find(keyName);

	if (itor != m_data.end())
		{
			//bingo!
			return &(itor->second);
		}
	return NULL; //doesn't exist
}

string DataManager::Get(const string &keyName)
{
	DataObject *pData = FindDataByKey(keyName);

	if (!pData) return "";
	return pData->Get();
}

float DataManager::GetNum(const string &keyName)
{
	DataObject *pData = FindDataByKey(keyName);

	if (!pData) return 0;
	
	return CL_String::to_float(pData->Get());
}


//returns true if we actually set the value, which we only do if it didn't exist
bool DataManager::SetIfNull(const string &keyName, const string &value)
{
	DataObject *pData = FindDataByKey(keyName);

	if (pData) return false; //didn't need to set it

	DataObject d;
	d.Set(keyName, value);
	m_data[keyName]=d;
	return true; //true that we created a key
}

bool DataManager::Exists(const string &keyName)
{
	return FindDataByKey(keyName) != 0;
}

void DataManager::Delete(const string &keyName)
{
	m_data.erase(keyName);
}


bool DataManager::Set(const string &keyName, const string &value)
{
	DataObject *pData = FindDataByKey(keyName);

	if (!pData)
	{
		//we need to create it too it looks like
		DataObject d;
		d.Set(keyName, value);
		m_data[keyName] = d;
		return true; //true that we created a key
	}

	pData->m_value = value;
	pData->m_dataType = E_STRING; //in case it was a num before
	
	return false; //false indicating that we didn't need to create a new key
}


bool DataManager::SetNum(const string &keyName, float num)
{
	DataObject *pData = FindDataByKey(keyName);

	if (!pData)
	{
		//we need to create it too it looks like
		DataObject d;
		d.SetNum(keyName, num);
		m_data[keyName] = d;
		return true; //true that we created a key
	}

	
	pData->m_num = num;
	pData->m_dataType = E_NUM; 
	return false; //false indicating that we didn't need to create a new key
}


void DataManager::Serialize(CL_FileHelper &helper)
{
	//load/save needed data
	int size = m_data.size();

	helper.process(size); //load or write it, depends

	if (helper.IsWriting())
	{
		dataList::iterator itor = m_data.begin();
		while (itor != m_data.end())
		{
			itor->second.Serialize(helper);
			itor++;
		}
	} else
	{
		//reading
		for (int i=0; i < size; i++)
		{
			DataObject o;
			o.Serialize(helper);
			m_data[o.m_key] = o;
		}
	}

	
}

/*
Object: DataManager
An object designed to flexibly store and retrieve numbers and strings.

In addition to any <Entity> having its own unique <DataManager> via <Entity::Data> a global one is always available through 


Group: Member Functions

func: Clear
(code)
nil Clear()
(end)
Deletes all data in the object.

*/