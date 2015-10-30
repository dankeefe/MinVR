
#include <net/VRNetClient.h>
#include <event/VRByteData.h>


int main() {
  
  if (VRByteData::isLittleEndian()) {
    std::cout << "little endian machine" << std::endl;
  }
  else {
    std::cout << "big endian machine" << std::endl;
  }
  
  
  VRNetClient client("localhost", "3490");
  int i = 0;
  std::vector<VREvent> events;
  while (1) {
    std::cout << "in draw loop " << i << std::endl;
    client.synchronizeInputEventsAcrossAllNodes(events);
    #ifdef WIN32
	  Sleep(2000);
    #else
      sleep(2);
    #endif
	client.synchronizeSwapBuffersAcrossAllNodes();
    i++;
  }
}