#include "internal.hpp"
#include "opcua_adapter.hpp"
#include "yaml.h"
#include <iostream>
#include <fstream>

#ifdef WIN32
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
#endif

using namespace std;

OpcuaAdapter::OpcuaAdapter(int aPort)
  : Adapter(aPort, 1000)
{
  mConnected = 0;

  printf("parsing yml\n");

  std::ifstream fin("opcua.yaml");
  YAML::Parser parser(fin);
  YAML::Node doc;
  parser.GetNextDocument(doc);
  const YAML::Node &comms = doc["communications"];
  comms["name"]		 >> mName;
  comms["url"]		 >> mUrl;
  comms["username"]  >> mUsername;
  comms["password"]  >> mPassword;
  comms["namespace"] >> mNamespace;

  printf("end 1 step\n");

  // Parse data
  const YAML::Node &data = doc["data"];  
  for(unsigned i = 0; i < data.size(); i++) {
	const YAML::Node &node = data[i];
	OpcuaData *data = NULL;
	std::string identifier;
	std::string type;
	std::string name;

	node["type"] >> type;
	node["identifier"] >> identifier;
	node["name"] >> name;

	printf("type: %s\n", type.c_str());
	printf("identifier: %s\n", identifier.c_str());

	if (type == "bool") {
		data = new OpcuaBool(identifier);
	} else if (type == "byte") {
		data = new OpcuaByte(identifier);
	} else if (type == "int16") {
		data = new OpcuaInt16(identifier);
	} else if (type == "int32") {
		data = new OpcuaInt32(identifier);
	} else if (type == "int64") {
		data = new OpcuaInt64(identifier);
	} else if (type == "float") {
		data = new OpcuaFloat(identifier);
	} else if (type == "double") {
		data = new OpcuaDouble(identifier);
	} else if (type == "string") {
		data = new OpcuaString(identifier);
	} else {
		printf("Invalid Opc Ua Type %s\n", type.c_str());
		exit(1);
	}

	//set data value
	data->createDataItem(name);
	//data->prefixName(mName);

	addDatum(*data->mDataItem);
	mData.push_back(data);

  }

  unavailable();

  printf("end 2 step\n");

}

OpcuaAdapter::~OpcuaAdapter()
{
	UA_Client_delete(client);
}

void OpcuaAdapter::initialize(int aArgc, const char *aArgv[])
{
  MTConnectService::initialize(aArgc, aArgv);
  if (aArgc > 1) {
    mPort = atoi(aArgv[0]);
  }
}

void OpcuaAdapter::start()
{
  startServer();
}

void OpcuaAdapter::stop()
{
  stopServer();
}

void OpcuaAdapter::gatherDeviceData()
{
  //check connection
  if (!mConnected) {
	int retval = UA_Client_connect(client, mUrl.c_str());
	if (retval != UA_STATUSCODE_GOOD) {
	  UA_Client_delete(client);
	  printf("ERROR Connection failed\n");
	  sleep(5);
	  return;
	}
	else {
		printf("Connection OK\n");
		mConnected = true;
	}
  }

  //read data
  std::vector<OpcuaData*>::iterator iter;
  for (iter = mData.begin(); iter != mData.end(); iter++) {
	  bool success = false;
	
	  char* identifier = strdup((*iter)->identifier().c_str());

	  printf("\nReading the value of node %s:\n", identifier);
	  UA_Variant *val = UA_Variant_new();
	  int retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(mNamespace, identifier), val);

	  if (retval == UA_STATUSCODE_GOOD && UA_Variant_isScalar(val)) {
		  success = true;
		  if ((*iter)->type() == OpcuaData::eBOOL) {
			  bool value = *(UA_Boolean*)val->data;
			  (static_cast<OpcuaBool*>(*iter))->setData(value);
			  printf("%s double value is: %f\n", identifier, value);
		  }
		  else if ((*iter)->type() == OpcuaData::eBYTE) {
			  uint8_t value = *(UA_Byte*)val->data;
			  (static_cast<OpcuaByte*>(*iter))->setData(value);
			  printf("%s byte value is: %d\n", identifier, value);
		  }
		  else if ((*iter)->type() == OpcuaData::eINT16) {
			  short value = *(UA_Int16*)val->data;
			  (static_cast<OpcuaInt16*>(*iter))->setData(value);
			  printf("%s int16 value is: %d\n", identifier, value);
		  }
		  else if ((*iter)->type() == OpcuaData::eINT32) {
			  int value = *(UA_Int32*)val->data;
			  (static_cast<OpcuaInt32*>(*iter))->setData(value);
			  printf("%s int32 value is: %d\n", identifier, value);
		  }
		  else if ((*iter)->type() == OpcuaData::eINT64) {
			  long value = *(UA_Int64*)val->data;
			  (static_cast<OpcuaInt64*>(*iter))->setData(value);
			  printf("%s int64 value is: %d\n", identifier, value);
		  }
		  else if ((*iter)->type() == OpcuaData::eFLOAT) {
			  float value = *(UA_Float*)val->data;
			  (static_cast<OpcuaFloat*>(*iter))->setData(value);
			  printf("%s float value is: %f\n", identifier, value);
		  }
		  else if ((*iter)->type() == OpcuaData::eDOUBLE) {
			  double value = *(UA_Double*)val->data;
			  (static_cast<OpcuaDouble*>(*iter))->setData(value);
			  printf("%s double value is: %f\n", identifier, value);
		  }
		  else if ((*iter)->type() == OpcuaData::eSTRING) {
			  UA_String temp = *(UA_String*)val->data;
			  std::string value = std::string((const char*)temp.data);
			  value = value.substr(0, temp.length);
			  UA_String_deleteMembers(&temp);
			  printf("%s string value is: %s\n", identifier, value.c_str());
		  }
		  else {
			  success = false;
		  }
	  }

	  if (success) {
		  (*iter)->writeValues();
	  }
	  else {
		  (*iter)->unavailable();
	  }

	  UA_Variant_delete(val);
	  free(identifier);

  }


  /*std::vector<OpcuaDevice>::iterator device;
  for (device = mDevices.begin(); device != mDevices.end(); device++) {
    std::vector<OpcuaData*>::iterator iter;
    for (iter = device->mData.begin(); iter != device->mData.end(); iter++) {
      bool success = false;
      if ((*iter)->type() == OpcuaData::eCOIL) {
        success = read_input_status(&mb_param, device->mAddress, (*iter)->address(), (*iter)->count(),
                                  (static_cast<OpcuaCoil*>(*iter))->data()) > 0;
      } else { 
        success = read_holding_registers(&mb_param, device->mAddress, (*iter)->address(), (*iter)->count(),
                                         (static_cast<OpcuaRegister*>(*iter))->data()) > 0;
        if (success && (*iter)->type() == OpcuaData::eDOUBLE) {
          OpcuaDouble *d = static_cast<OpcuaDouble*>(*iter);
          if (d->scalerAddress() != 0) {
            uint16_t scaler[1];
            success = read_holding_registers(&mb_param, device->mAddress, d->scalerAddress(), 1, scaler) > 0;
            if (success) d->scaler = scaler[0];
          }
        }
      }

      if (success) {
        (*iter)->writeValues();
      } else {
        (*iter)->unavailable();
      }      
    }
    
    flush();
  }*/
}

