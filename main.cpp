#include <iostream>
#include <pulse/pulseaudio.h>
#include <cstring>
#include <vector>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>

/*
 * Тип данных показывающий ошибки
 */

enum class PulseErrors {
  kNoError,
  kCreateMainLoop,
  kCreateMainLoopApi,
  kCreateContext,
  kContextConnect,
  kBufferRecord
};

PulseErrors error = PulseErrors::kNoError;

/*
 * Глобальный цикл, связывающийся с сервером pulseaudio
 */
 pa_threaded_mainloop* mainloop;

//----------------------------------------------------------------------------------------------------------------------

/*
 * Коллбэк для изменения состояния контекста(его подключения)
 */

void on_state_change_context(pa_context *c, void *userdata) {
  std::cout << "State changed" << std::endl;
  pa_threaded_mainloop_signal(mainloop, 0);
}

//----------------------------------------------------------------------------------------------------------------------

/*
 * Коллбэк для обнаружения устройства вывода
 */

void on_dev_sink(pa_context *c, const pa_sink_info *info, int eol, void *udata)
{
  std::vector<std::string>* devices = static_cast<std::vector<std::string>*>(udata);
  if (eol != 0) {
    pa_threaded_mainloop_signal(mainloop, 0);
    return;
  }
  const char *device_id = info->name;
  devices->push_back(info->name);
}

/*
 * Коллбэк для изменения состояния аудиобуффера(когда в него можно записывать новые данные)
 */

void on_io_complete(pa_stream *s, size_t nbytes, void *udata)
{
  pa_threaded_mainloop_signal(mainloop, 0);
  std::cout << "Audiobuffer" << std::endl;
}

//----------------------------------------------------------------------------------------------------------------------
/*
 * Создает контекст для связи с аудио сервером pulseaudio
 */

pa_context* create_client_context() {
  mainloop = pa_threaded_mainloop_new();
  if (mainloop == NULL) {
    error = PulseErrors::kCreateMainLoop;
    return NULL;
  }

  pa_mainloop_api* mainloop_api = pa_threaded_mainloop_get_api(mainloop);
  if (mainloop_api == NULL) {
    pa_threaded_mainloop_free(mainloop);
    error = PulseErrors::kCreateMainLoopApi;
    return NULL;
  }

  //TODO возможно тут и далее возникает утечка памяти, так как созданный ранее api не удаляется
  pa_context* context = pa_context_new_with_proplist(mainloop_api, "audio_client", NULL);
  if (context == NULL) {
    pa_threaded_mainloop_free(mainloop);
    error = PulseErrors::kCreateContext;
    return NULL;
  }
  if (0 != pa_context_connect(context, NULL, static_cast<pa_context_flags_t>(0), NULL)) {
    pa_threaded_mainloop_free(mainloop);
    pa_context_unref(context);
    error = PulseErrors::kCreateContext;
    return NULL;

  }
  void *udata = NULL;
  pa_context_set_state_callback(context, on_state_change_context, udata);

  pa_threaded_mainloop_start(mainloop);

  pa_threaded_mainloop_lock(mainloop);
  for (;;) {
    // Check the current state of the ongoing connection
    int r = pa_context_get_state(context);
    if (r == PA_CONTEXT_READY) {
      break;
    } else if (r == PA_CONTEXT_FAILED || r == PA_CONTEXT_TERMINATED) {
      error = PulseErrors::kContextConnect;
      pa_context_unref(context);
      pa_threaded_mainloop_stop(mainloop);
      pa_threaded_mainloop_free(mainloop);
      assert(0);
    }

    // Not yet connected. Block execution until some signal arrives.
    pa_threaded_mainloop_wait(mainloop);
  }
  pa_threaded_mainloop_unlock(mainloop);

  return context;
}

//----------------------------------------------------------------------------------------------------------------------

/*
 * Получение списка доступных устройств
 */
void get_available_sinks(pa_context* context,  std::vector<std::string>* sinks) {
  pa_operation *op;  // операция исполняемая над функцией обратного вызова
  void *udata = sinks;

  pa_threaded_mainloop_lock(mainloop);
  op = pa_context_get_sink_info_list(context, on_dev_sink, udata);

  for (;;) {
    int r = pa_operation_get_state(op);
    if (r == PA_OPERATION_DONE || r == PA_OPERATION_CANCELLED)
      break;
    pa_threaded_mainloop_wait(mainloop);
  }

  pa_operation_unref(op);
  pa_threaded_mainloop_unlock(mainloop);
}

//----------------------------------------------------------------------------------------------------------------------

/*
 * Создание аудиобуффера(аудиобуффер - место откуда непосредственно будет считываться звук)
 */

pa_stream* create_audiobuffer_playback(pa_context* context, const char *device_id) {
  if (device_id == NULL) {
    std::cout << "The name of the device was not received" << std::endl;
    return NULL;
  }
  pa_sample_spec spec;  //  по сути формат и спецификация аудиофайла
  spec.format = PA_SAMPLE_S16LE;
  spec.rate = 48000;  // частота дискретизации
  spec.channels = 2;  // двухканальный звук

  pa_threaded_mainloop_lock(mainloop);
  pa_stream *stm = pa_stream_new(context, "audio-client", &spec, NULL);  // сам аудиобуффер

  pa_buffer_attr attr;  // настройки аудиобуффера
  memset(&attr, 0xff, sizeof(attr));
  /* Значение 500 выбрано как некоторый средний вариант, на самом деле нет объективных причин почему это должно быть
   * именно 500, а не, к примеру 400 или 450. Если это значени будет слишком большим, то получится система с достаточно
   * большой задержкой. Если же сильно уменьшить значение, то возрастет нагрузка на устройство, а некоторые устройства
   * вообще перестанут поддерживать такой буффер
   */
  int buffer_length_msec = 500;
  /* Человеку проще воспринимать размер буффера в мс, но для компьютера это значение должно быть представленно в
   * байтах. Частота дискретезации по сути отвечает за то сколько точек в секунду будет сниматься с аналогового сигнала.
   * Число 16 - говорит о 16-битном звуке, то есть одна точка представляет собой число от 0 до 65535. Таким образом получается
   * количество байт, которые занимает точка снятая с аналогового сигнала. И наконец полученное значение умножается на
   * максимальный размер музыкального отрезка в секундах, который может содержаться в буффере. На выходе получается
   * размер буффера в байтах.
   */
  attr.tlength = spec.rate * 16/8 * spec.channels * buffer_length_msec / 1000;
  void *udata = NULL;
  pa_stream_set_write_callback(stm, on_io_complete, udata);
  pa_stream_connect_playback(stm, device_id, &attr, static_cast<pa_stream_flags_t>(0), NULL, NULL);
  for (;;) {
    int r = pa_stream_get_state(stm);
    if (r == PA_STREAM_READY)
      break;
    else if (r == PA_STREAM_FAILED) {
      error = PulseErrors::kBufferRecord;
      return NULL;
    }
  pa_threaded_mainloop_wait(mainloop);
  }

  pa_threaded_mainloop_unlock(mainloop);

  return stm;
}

//----------------------------------------------------------------------------------------------------------------------

/*void write_data_to_buffer(pa_stream* audiobuffer, void* audio_sample_data) {
  size_t n = pa_stream_writable_size(audiobuffer);
  if (n == 0) {
    pa_threaded_mainloop_wait(mainloop);
    continue;
  }
}*/

//----------------------------------------------------------------------------------------------------------------------

std::vector<char> get_file_byte_array(const std::string& path_to_file) {
  FILE* fd = fopen(path_to_file.c_str(), "r");

  if (fd == NULL) {
    std::cout << path_to_file << " not exist" << std::endl;
  }

  fseek(fd, 0, SEEK_END); // seek to end of file
  size_t size_of_file = ftell(fd); // get current file pointer
  std::cout << size_of_file << std::endl;
  rewind(fd); // seek back to beginning of file
  std::vector<char> raw_data_from_file;
  raw_data_from_file.resize(size_of_file);
  size_t flag = fread(raw_data_from_file.data(), sizeof(char), size_of_file, fd);

  // std::cout <<  << std::endl;
  return raw_data_from_file;
}

//----------------------------------------------------------------------------------------------------------------------

void delete_audiobuffer(pa_stream* stream) {
  pa_threaded_mainloop_lock(mainloop);
  pa_stream_disconnect(stream);
  pa_stream_unref(stream);
  pa_threaded_mainloop_unlock(mainloop);
}
//----------------------------------------------------------------------------------------------------------------------

/*
 * По сути очищает память просто и всё
 */

void delete_client_context(pa_context* context) {
  pa_threaded_mainloop_lock(mainloop);
  pa_context_disconnect(context);
  pa_context_unref(context);
  pa_threaded_mainloop_unlock(mainloop);

  pa_threaded_mainloop_stop(mainloop);
  pa_threaded_mainloop_free(mainloop);
}


int main() {
  std::cout << "1" << std::endl;
  pa_context* client_context = create_client_context();
  std::vector<std::string> devices;
  get_available_sinks(client_context, &devices);

  for (auto& i : devices)
    std::cout << i << std::endl;

  pa_stream* audiobuffer_playback = create_audiobuffer_playback(client_context, devices[1].c_str());

  std::vector<char> audiofile = get_file_byte_array("/home/galkin/Рабочий стол/Projects/audio_palyer/audio/sample-12s.wav");
  std::cout << "Size of file: " << audiofile.size() << std::endl;
  delete_audiobuffer(audiobuffer_playback);
  delete_client_context(client_context);
  std::cout << "2" << std::endl;
  return 0;
}
