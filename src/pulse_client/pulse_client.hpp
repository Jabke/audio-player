#ifndef PULSE_CLIENT_HPP_ 
#define PULSE_CLIENT_HPP_
#include <pulse/pulseaudio.h>
#include <vector>
#include <string>
#include <memory>

//----------------------------------------------------------------------------------------------------------------------

using SinksInformation = std::vector<pa_sink_info>;

//----------------------------------------------------------------------------------------------------------------------

class PulseClient {
  public:
    PulseClient(std::string application_name);
    ~PulseClient();
    PulseClient& operator=(const PulseClient&);
    PulseClient(const PulseClient&);

    SinksInformation GetAvailableSinks();
    // pa_sample_spec - format of sample: frequency, size, type etc
    bool SetSampleSettings(const pa_sample_spec& spec);
    void CreateAudioBuffer();

};

#endif // PULSE_CLIENT_HPP_