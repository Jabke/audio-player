#ifndef PLAYER_HPP_
#define PLAYER_HPP_
#include <string>

namespace audio_player {

enum class PlaybackModes {
  kOnlyCurrentTrack,  // Only the current track is played, after which playback stops
  kAllTracksInFolder  // All tracks from the folder are played, after which playback stops
};

class AudioPlayer {
 public:
  AudioPlayer();
  AudioPlayer(AudioPlayer&);
  ~AudioPlayer();
  AudioPlayer& operator=(const AudioPlayer&);

  void Start(std::string path_to_audio);
  void Stop();  // Stopping current track
  void Pause();  // Pausing current track
  void SetPlaybackMode();  //

};

};  // audio_player

#endif  // PLAYER_HPP_
