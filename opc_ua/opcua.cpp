

#include "internal.hpp"
#include "opcua_adapter.hpp"
#include "server.hpp"
#include "string_buffer.hpp"

int main(int aArgc, const char *aArgv[])
{    
  /* Construct the adapter and start the server */
  OpcuaAdapter *adapter = new OpcuaAdapter(7878);
  adapter->setName("MTConnect Opcua Adapter");
  return adapter->main(aArgc, aArgv);
}

