
#ifndef OPCUA_ADAPTER_HPP
#define OPCUA_ADAPTER_HPP

#include "adapter.hpp"
#include "service.hpp"
#include "device_datum.hpp"
#include "open62541.h"
#include <string>
#include <vector>

class OpcuaData {
public:
  typedef enum {
    eBOOL,
    eBYTE,
	eINT16,
	eINT32,
	eINT64,
    eFLOAT,
	eDOUBLE,
	eSTRING
  } tType;
    
public:
  OpcuaData(std::string a) : mIdentifier(a) { }
  
  DeviceDatum *mDataItem;

  void prefixName(std::string &aName) {
	mDataItem->prefixName(aName.c_str());
  }

  std::string identifier() { return mIdentifier; }
  tType type() { return mType; }

  virtual void createDataItem(std::string &aName) = 0;
  virtual void writeValues() = 0;

  void unavailable() {
	mDataItem->unavailable();
  }
   
protected:
  std::string mIdentifier;
  tType mType;
};

class OpcuaBool : public OpcuaData {
public:
	OpcuaBool(std::string a) : OpcuaData(a) { mType = eBOOL; mData = 0; }

	~OpcuaBool() {
		mData = 0;
	}

	virtual void createDataItem(std::string &aNames) {
		mDataItem = new IntEvent(aNames.c_str());
	}

	virtual void writeValues() {
		(static_cast<IntEvent*>(mDataItem))->setValue(mData);
	}

	void setData(bool value) { mData = value; }

protected:
	bool mData;
};

class OpcuaByte : public OpcuaData {
public:
	OpcuaByte(std::string a) : OpcuaData(a) { mType = eBYTE; mData = 0; }

	~OpcuaByte() {
		mData = 0;
	}

	virtual void createDataItem(std::string &aNames) {
		mDataItem = new IntEvent(aNames.c_str());
	}

	virtual void writeValues() {
		(static_cast<IntEvent*>(mDataItem))->setValue(mData);
	}

	void setData(uint8_t value) { mData = value; }

protected:
	uint8_t mData;
};

class OpcuaInt16 : public OpcuaData {
public:
	OpcuaInt16(std::string a) : OpcuaData(a) { mType = eINT16; mData = 0; }

	~OpcuaInt16() {
		mData = 0;
	}

	virtual void createDataItem(std::string &aNames) {
		mDataItem = new IntEvent(aNames.c_str());
	}

	virtual void writeValues() {
		(static_cast<IntEvent*>(mDataItem))->setValue(mData);
	}

	void setData(short value) { mData = value; }

protected:
	short mData;
};

class OpcuaInt32 : public OpcuaData {
public:
	OpcuaInt32(std::string a) : OpcuaData(a) { mType = eINT32; mData = 0; }

	~OpcuaInt32() {
		mData = 0;
	}

	virtual void createDataItem(std::string &aNames) {
		printf("createItem %s\n", aNames.c_str());
		mDataItem = new IntEvent(aNames.c_str());
	}

	virtual void writeValues() {
		(static_cast<IntEvent*>(mDataItem))->setValue(mData);
		printf("writeValues %d\n", mData);
	}

	void setData(int value) { mData = value; printf("setData %d\n", value);  }

protected:
	int mData;
};

class OpcuaInt64 : public OpcuaData {
public:
	OpcuaInt64(std::string a) : OpcuaData(a) { mType = eINT64; mData = 0; }

	~OpcuaInt64() {
		mData = 0;
	}

	virtual void createDataItem(std::string &aNames) {
		mDataItem = new IntEvent(aNames.c_str());
	}

	virtual void writeValues() {
		(static_cast<IntEvent*>(mDataItem))->setValue(mData);
	}

	void setData(long value) { mData = value; }

protected:
	long mData;
};

class OpcuaFloat : public OpcuaData {
public:
	OpcuaFloat(std::string a) : OpcuaData(a) { mType = eFLOAT; mData = 0; }

	~OpcuaFloat() {
		mData = 0;
	}

	virtual void createDataItem(std::string &aNames) {
		mDataItem = new Sample(aNames.c_str());
	}

	virtual void writeValues() {
		(static_cast<Sample*>(mDataItem))->setValue(mData);
	}

	void setData(float value) { mData = value; }

protected:
	float mData;
};

class OpcuaDouble : public OpcuaData {
public:
	OpcuaDouble(std::string a) : OpcuaData(a) { mType = eDOUBLE; mData = 0; }

	~OpcuaDouble() {
		mData = 0;
	}

	virtual void createDataItem(std::string &aNames) {
		mDataItem = new Sample(aNames.c_str());
	}

	virtual void writeValues() {
		(static_cast<Sample*>(mDataItem))->setValue(mData);
	}

	void setData(double value) { mData = value; }

protected:
	double mData;
};

class OpcuaString : public OpcuaData {
public:
	OpcuaString(std::string a) : OpcuaData(a) { mType = eSTRING; mData = ""; }

	~OpcuaString() {
		mData = "";
	}

	virtual void createDataItem(std::string &aNames) {
		mDataItem = new Event(aNames.c_str());
	}

	virtual void writeValues() {
		(static_cast<Event*>(mDataItem))->setValue(mData.c_str());
	}

	void setData(std::string value) { mData = value; }

protected:
	std::string mData;
};


class OpcuaAdapter : public Adapter, public MTConnectService
{
protected:

  int mConnected;

  std::string mName;
  std::string mUrl;
  std::string mUsername;
  std::string mPassword;
  int mNamespace;
  
  std::vector<OpcuaData*> mData;

  UA_Client *client = UA_Client_new(UA_ClientConfig_standard);

public:
  OpcuaAdapter(int aPort);
  ~OpcuaAdapter();
  
  virtual void initialize(int aArgc, const char *aArgv[]);
  virtual void start();
  virtual void stop();
  
  virtual void gatherDeviceData();
};

#endif

