#ifndef PULSE_CLIENT_HPP_ 
#define PULSE_CLIENT_HPP_
#include <pulse/pulseaudio.h>
#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace pulse_client {

//----------------------------------------------------------------------------------------------------------------------

using SinksInformation = std::vector<pa_sink_info>;

//----------------------------------------------------------------------------------------------------------------------

using ContextCallBacks = std::function<void(pa_context*, void*)>;

/***********************************************************************************************************************
 *** Pulse audio facade
 *(there lot pulse struct have incomplete type, it takes away oppotunity to normaly work with these types)
 **********************************************************************************************************************/

//---PAThreadedMainLoop---------------------------------------------------------------------------------------------------

class PAThreadedMainLoop {
 public:
  PAThreadedMainLoop();
  PAThreadedMainLoop& operator=(const PAThreadedMainLoop&) = delete;
  PAThreadedMainLoop(const PAThreadedMainLoop&) = delete;
  ~PAThreadedMainLoop();
  pa_threaded_mainloop* GetRawPointer();
  pa_mainloop_api* GetMainLoopAPI();

 private:
  pa_threaded_mainloop* mainloop_ = NULL;
  pa_mainloop_api* pa_mainloop_api_ = NULL;
};

//---PAContext----------------------------------------------------------------------------------------------------------
//
class PAContext {
 public:
  PAContext() = delete;
  PAContext(PAThreadedMainLoop&, std::string&);
  PAContext& operator=(const PAContext&) = delete;
  PAContext(const PAContext&) = delete;
  ~PAContext();
  pa_context* GetRawPointer();
  pa_threaded_mainloop* GetRawLoopPointer();
  void SynchroningConnect();

  static void ContextCallBackWrapper(pa_context *c, void *userdata);

 private:
  pa_context* connection_context_ = NULL;
  pa_mainloop_api* pa_mainloop_api_ = NULL;
  pa_threaded_mainloop* mainloop_ = NULL;
  std::string context_user_name_;

};

//---PAOperation----------------------------------------------------------------------------------------------------------

/***********************************************************************************************************************
 *** Pulse audio client
***********************************************************************************************************************/

class PulseClient {
 public:
  PulseClient(std::string application_name);
  ~PulseClient();
  PulseClient &operator=(const PulseClient &);
  PulseClient(const PulseClient &);

  SinksInformation GetAvailableSinks();
  // pa_sample_spec - format of sample: frequency, size, type etc
  bool SetSampleSettings(const pa_sample_spec &spec);
  void CreateAudioBuffer();

 private:
  std::string application_name_;

  // pulse entities
  PAThreadedMainLoop mainloop_;

};

};  // pulse_client
#endif // PULSE_CLIENT_HPP_