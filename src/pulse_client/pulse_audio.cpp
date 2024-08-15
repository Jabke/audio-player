//
// Created by galkin on 15.08.24.
//
#include "pulse_client.hpp"
#include "exceptions.h"
#include <pulse/thread-mainloop.h>
#include <iostream>

namespace pulse_client {

/***********************************************************************************************************************
 *** Pulse audio facade
***********************************************************************************************************************/

//---MAINLOOP-----------------------------------------------------------------------------------------------------------

PAThreadedMainLoop::PAThreadedMainLoop() {
  mainloop_ = pa_threaded_mainloop_new();
  if (mainloop_ == NULL) {
    throw pulse_exceptions::ExceptionWhenCreatingPAFacade("PAThreadedMainLoop");
  }
}

//----------------------------------------------------------------------------------------------------------------------

PAThreadedMainLoop::~PAThreadedMainLoop() {
   pa_threaded_mainloop_free(mainloop_);
}

//----------------------------------------------------------------------------------------------------------------------

pa_threaded_mainloop* PAThreadedMainLoop::GetRawPointer() {
  return mainloop_;
}

//----------------------------------------------------------------------------------------------------------------------

pa_mainloop_api* PAThreadedMainLoop::GetMainLoopAPI() {
  return pa_threaded_mainloop_get_api(mainloop_);
}

//---CONTEXT-----------------------------------------------------------------------------------------------------------

PAContext::PAContext(PAThreadedMainLoop& main_loop, std::string& context_user_name) :
  pa_mainloop_api_(main_loop.GetMainLoopAPI()), context_user_name_(context_user_name),
  mainloop_(main_loop.GetRawPointer()) {
  connection_context_ = pa_context_new_with_proplist(pa_mainloop_api_, context_user_name_.c_str(), NULL);
  if (connection_context_ == NULL) {
    throw pulse_exceptions::ExceptionWhenCreatingPAFacade("Context");
  }
}

//----------------------------------------------------------------------------------------------------------------------

pa_context* PAContext::GetRawPointer() {
  return connection_context_;
}

//----------------------------------------------------------------------------------------------------------------------

pa_threaded_mainloop* PAContext::GetRawLoopPointer() {
  return mainloop_;
}

//---Callback-----------------------------------------------------------------------------------------------------------

void PAContext::ContextCallBackWrapper(pa_context *c, void *userdata) {
  std::cout << "CONNECTION IS GOOD" << std::endl;
  pa_threaded_mainloop_signal(static_cast<PAContext*>(userdata)->GetRawLoopPointer(), 0);
}

//----------------------------------------------------------------------------------------------------------------------

void PAContext::SynchroningConnect() {
  // NULL - default server, 0 - no specific options, NULL - ?
  if (0 != pa_context_connect(connection_context_, NULL, static_cast<pa_context_flags_t>(0), NULL)) {
    throw pulse_exceptions::BaseException("Error of creating connection");
  }
  void* udata = this;  // no need pass somthing to callback
  pa_context_set_state_callback(connection_context_, ContextCallBackWrapper, udata);

  pa_threaded_mainloop_lock(mainloop_);

  while (true) {
    // Check the current state of the ongoing connection
    int r = pa_context_get_state(connection_context_);
    if (r == PA_CONTEXT_READY) {
      break;
    } else if (r == PA_CONTEXT_FAILED || r == PA_CONTEXT_TERMINATED) {
      throw pulse_exceptions::BaseException("Error of context when trying to connect");
    }
    pa_threaded_mainloop_wait(mainloop_);
  }
  pa_threaded_mainloop_unlock(mainloop_);
}

//----------------------------------------------------------------------------------------------------------------------

PAContext::~PAContext() {
  pa_threaded_mainloop_lock(mainloop_);
  pa_context_disconnect(connection_context_);
  pa_context_unref(connection_context_);
  pa_threaded_mainloop_unlock(mainloop_);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

  PulseClient::PulseClient(std::string application_name) : application_name_(application_name) {

  }
};  // pulse_client