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
An object designed to flexibly store and retrieve numbers and strings very quickly use key/value pairs.

In addition to any <Entity> having its own unique <DataManager> via <Entity::Data> a global one is always available through <GameLogic::Data> as well.

Using the Entity Properties Editor, you can also add/remove/modify data from the editor.

Group: Storing Data

func: Set
(code)
boolean Set(string keyName, string value)
(end)
Store a string as a named piece of data to be retrieved later.
Replaces data by this key name if it already exists.

keyName - Any name you wish.
value - A string of any length with the text/data that you wish to store.

Returns:

True if a new piece of data was created, false if existing data was replaced.

func: SetNum
(code)
boolean SetNum(string keyName, number num)
(end)
Similar to <Set> but saves the data as a number which saves space and is faster to access internally.

When <Get>is used with a number (instead of <GetNum>) it is automatically converted into a string.

Decimal points are ok to use.  The accuracy maintained is that of a C "float".

keyName - Any name you wish.
num - The number you wish to store.

Returns:

True if a new piece of data was created, false if existing data was replaced.

func: SetIfNull
(code)
boolean SetIfNull(string keyName, string value)
(end)
Similar to <Set> but stores the data only if the key didn't already exist.

When <Get>is used with a number (instead of <GetNum>) it is automatically converted into a string.

keyName - Any name you wish.
value - The string data you wish to store.

Returns:

True if the key didn't exist and the value was stored.  False if it already existed and nothing was changed.

Group: Retrieving Data

func: Get
(code)
string Get(string keyName)
(end)
Retrieve a previously stored value by its key name.

keyName - The key-name used when it was stored.

Returns:

The data in string form or a blank string if the key wasn't found.  Use <Exists> to verify if a key exists or not.

func: GetNum
(code)
number Get(string keyName)
(end)
Like <Get> but returns the data as a number.  If the data was stored as a number, this is the fastest way to access it.

If the data was stored as a string, an attempt to convert it to a number is made.

keyName - The key-name used when it was stored.

Returns:

The number that was stored.

Group: Miscellaneous

func: Exists
(code)
boolean Exists(string keyName)
(end)

keyName - The key name used when it was stored.

Returns:

True if data with this key name exists.

func: ModNum
(code)
number Exists(string keyName, number modAmount)
(end)
Modifies an existing key by a number.  Creates the key if it didn't exist.

Usage:
(code)

//set hitpoints to 100
this:Data():SetAsNum("life", 100);

//remove 5
local curHitpoints = this:Data():ModNum("life", -5);

LogMsg("I only have " .. curHitpoints .. " hitpoints!");
(end)

keyName - The key name used when it was stored.
modAmount - How much the number should be changed by.

Returns:

The new number that was stored.

func: Delete
(code)
nil Delete(string keyName)
(end)
Completely removes a key/value pair from the database.

keyName - The key name used when it was stored.

func: Clear
(code)
nil Clear()
(end)
Completely removes all stored data from this database.
*/


