#pragma once

#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/collider.hpp>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>
#include <AL/efx-presets.h>
#include <AL/efx-creative.h>

#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

namespace PrettyEngine {
	static bool SUPPORT_OPENAL_EXTENSION_EFX = false;

	static inline ALenum to_al_format(short channels, short samples)
	{
	        bool stereo = (channels > 1);

	        switch (samples) {
	        case 16:
	                if (stereo)
	                        return AL_FORMAT_STEREO16;
	                else
	                        return AL_FORMAT_MONO16;
	        case 8:
	                if (stereo)
	                        return AL_FORMAT_STEREO8;
	                else
	                        return AL_FORMAT_MONO8;
	        default:
	                return -1;
	        }
	}

	static std::vector<unsigned char> ReadAudioFile(std::string fileName) {
		std::ifstream fileStream(fileName, std::ios::binary);
	    if (!fileStream.is_open()) {
	        DebugLog(LOG_ERROR, "Error opening file: " << fileName, true);
	        std::exit(-1);
	    }

	    // Get the file size by seeking to the end and then telling the position
	    fileStream.seekg(0, std::ios::end);
	    std::streampos fileSize = fileStream.tellg();
	    fileStream.seekg(0, std::ios::beg);

	    // Allocate memory to store the file content
	    unsigned char* fileContent = new unsigned char[fileSize];

	    // Read the file content into the allocated memory
	    fileStream.read(reinterpret_cast<char*>(fileContent), fileSize);

	    // Check if the file was read successfully
	    if (!fileStream) {
	        DebugLog(LOG_ERROR, "Error reading file: " << fileName, true);
	        delete[] fileContent;
	        std::exit(-1);
	    }

	    // Close the file stream
	    fileStream.close();

	    std::vector<unsigned char> out;

	    for (int i = 0; i < fileSize; i++) {
	    	out.push_back(fileContent[i]);
	    }

	    delete[] fileContent; 

	    return out;
	}

	enum class AudioState {
		Play,
		Pause,
		Stop,
	};

	enum class AudioFormat {
		Mono8 = AL_FORMAT_MONO8,
		Mono16 = AL_FORMAT_MONO16,
		Stereo8 = AL_FORMAT_STEREO8,
		Stereo16 = AL_FORMAT_STEREO16,
	};

	/// Control the audio listener
	class AudioListener: virtual public Transform {
	public:
		/// Sync the threaded listener with the current values
		void UpdateAudioListener() {

			alListener3f(AL_POSITION, this->position.x, this->position.y, this->position.y);	

			float orientation[6] = { 
				this->listenerForwardVector.x, 
				this->listenerForwardVector.y, 
				this->listenerForwardVector.z, 
				this->listenerUpVector.x, 
				this->listenerUpVector.y, 
				this->listenerUpVector.z, 
			};

			alListenerfv(AL_ORIENTATION, orientation);
		}

	public:
		glm::vec3 listenerForwardVector = glm::vec3(0.0, 0.0f, -1.0f);
		glm::vec3 listenerUpVector = glm::vec3(0.0, 1.0f, 0.0f);
	};

	/// 3D Audio Source
	class AudioSource: virtual public Transform {
	public:
		AudioSource() {
			alGenSources(1, &this->openALSource);

			this->SetLooping(true);
		}

		~AudioSource() {
			this->Clear();
		}

		/// Stop and unload the audio
		void Clear() {
			this->Stop();
			if (this->openALSource != 0) {
				alDeleteSources(1, &this->openALSource);
			}
			if (this->openALBuffer != 0) {
				alDeleteBuffers(1, &this->openALBuffer);
			}
		}

		/// Update the threaded part of the audio source properties
		void UpdateAudioSourceProperties() {
			alSource3f(this->openALSource, AL_POSITION, this->position.x, this->position.y, this->position.z);
		    
		    alSourcef(this->openALSource, AL_REFERENCE_DISTANCE, this->radius);
		}

		/// Play the audio
		void Play() {
			alSourcePlay(this->openALSource);
		}

		/// Pause the audio
		void Pause() {
			alSourcePause(this->openALSource);
		}

		/// Stop the audio
		void Stop() {
			alSourceStop(this->openALSource);
		}

		/// Update the audio state and check for error
		void UpdateAudioSource() {
			this->UpdateAudioSourceProperties();

			if (audioState != nullptr) {
				switch(*audioState) {
					case AudioState::Play:
						this->Play();
						break;
					case AudioState::Pause:
						this->Pause();
						break;
					case AudioState::Stop:
						this->Stop();
						break;
				}
			}

			auto openAlError = alGetError();
			if (openAlError != AL_NO_ERROR) {
				DebugLog(LOG_DEBUG, "OpenAL Soft - Error: " << alGetError(), true);
			}
		}

		void SetLooping(bool value) {
			this->looping = value;
			alSourcei(this->openALSource, AL_LOOPING, value);
		}

		bool GetLooping() {
			return this->looping;
		}

		void SetGain(float newGain) {
			this->gain = newGain;
			alSourcei(this->openALSource, AL_GAIN, newGain);
		}

		float GetGain() {
			return this->gain;
		}

		void SetPitch(float newPitch) {
			this->pitch = newPitch;
			alSourcei(this->openALSource, AL_PITCH, newPitch);
		}

		float GetPitch() {
			return this->pitch;
		}

		void LoadFrequency(float frequency, float duration, float sampleRate) {
			if (this->openALBuffer != 0) {
				alSourcei(this->openALSource, AL_BUFFER, NULL);
			}

			if (this->openALBuffer == 0) {
				alGenBuffers(1, &this->openALBuffer);
			}

		    	const int numSamples = static_cast<int>(sampleRate * duration);
		    	float* samples = new float[numSamples];
		    
		    	for (int i = 0; i < numSamples; ++i) {
		        	samples[i] = 0.5f * std::sin(2.0f * glm::pi<float>() * frequency * static_cast<float>(i) / sampleRate);
		    	}

		    	alBufferData(this->openALBuffer, AL_FORMAT_MONO16, samples, numSamples * sizeof(float), sampleRate);

			alSourcei(this->openALSource, AL_BUFFER, this->openALBuffer);
		
			auto openAlError = alGetError();
			if (openAlError != AL_NO_ERROR) {
				DebugLog(LOG_DEBUG, "OpenAL Soft - Error: " << alGetError(), true);
			}
		}

		void LoadWAVAudio(std::string fileName) {
			if (this->openALBuffer != 0) {
				alSourcei(this->openALSource, AL_BUFFER, NULL);
			}

			if (this->openALBuffer == 0) {
				alGenBuffers(1, &this->openALBuffer);
			}

			drwav wav;
			if (!drwav_init_file(&wav, fileName.c_str(), NULL)) {
    			DebugLog(LOG_ERROR, "Failed to open audio file: " << fileName, true);
    			drwav_uninit(&wav);

			} else {
				drwav_uint64 totalFrames = wav.totalPCMFrameCount; // Get the total number of frames in the audio file

		    	if (wav.bitsPerSample == 16) {
		    		drwav_int16* pcmBuffer = (drwav_int16*)malloc(totalFrames * wav.channels * sizeof(drwav_int16)); // Allocate memory for the PCM buffer
			
					unsigned long long readedFrames = drwav_read_pcm_frames_s16(&wav, totalFrames, pcmBuffer);

					alBufferData(this->openALBuffer, to_al_format(wav.channels, wav.bitsPerSample), pcmBuffer, readedFrames * wav.channels * sizeof(drwav_int16), wav.sampleRate);

					alSourcei(this->openALSource, AL_BUFFER, this->openALBuffer);

					free(pcmBuffer);
				}
				
				auto alError = alGetError();
				if (alError != AL_NO_ERROR) {
					DebugLog(LOG_DEBUG, "OpenAL Soft - Error: " << alGetError(), false);
				}

				drwav_uninit(&wav);
				return;
			}
			drwav_uninit(&wav);
			DebugLog(LOG_ERROR, "Failed to load: " << fileName, true);
		}

		bool IsPlaying() {
			ALenum sourceState;
			alGetSourcei(this->openALSource, AL_SOURCE_STATE, &sourceState);
			return (sourceState == AL_PLAYING);
		}
		
	public:
		unsigned int buffer;
		float radius = 10.0f;
		AudioState* audioState = nullptr;

	private:
		float gain = 1.0f;
		float pitch = 1.0f;
		bool looping = false;

		unsigned int openALBuffer = 0;
		unsigned int openALSource = 0;
	};

	class AudioEngine {
	public:
		AudioEngine() {
			this->openALCurrentDevice = alcOpenDevice(NULL);

	    	this->UpdateContext();
	    	this->SetCurrent();

	    	const ALCchar* extensions = alcGetString(NULL, ALC_EXTENSIONS);
			if (extensions && strstr(extensions, "ALC_EXT_EFX") != NULL) {
				SUPPORT_OPENAL_EXTENSION_EFX = true;
			}
		}
		
		~AudioEngine() {
			for(auto & audioSource: this->sources) {
				audioSource->Clear();
			}
			
			if (this->openALContext != nullptr) {
				alcDestroyContext(this->openALContext);
			}
			if (this->openALCurrentDevice != nullptr) {
				alcCloseDevice(this->openALCurrentDevice);
			}
		}

		void UpdateContext() {
			if (this->openALContext != nullptr) {
				alcDestroyContext(this->openALContext);
			}

			this->SetAudioFrequency(this->GetCurrentAudioDeviceFrequency());
			this->SetMonoSources(this->GetCurrentAudioDeviceMonoSources());
			this->SetStereoSources(this->GetCurrentAudioDeviceStereoSources());
			this->SetAudioSync(true);
			this->SetAudioRefresh(this->GetCurrentAudioDeviceRefreshRate());

			if (this->CheckCurrentDevice()) {
				this->openALContext = alcCreateContext(this->openALCurrentDevice, this->attributes);
			}
		}

		void SetCurrent() {
			alcMakeContextCurrent(this->openALContext);
		}

		bool CheckCurrentDevice() {
			if (!this->openALCurrentDevice || this->openALCurrentDevice == nullptr || this->openALCurrentDevice == NULL) {
		        DebugLog(LOG_ERROR, "Failed to initialize OpenAL device, no audio devices found.", true);
		        return false;
		    }
		    return true;
		}

		void SetAudioDevice(std::string name) {
			if (this->openALCurrentDevice != nullptr) {
				std::cout << "Close device" << std::endl;
				alcCloseDevice(this->openALCurrentDevice);
			}

			this->openALCurrentDevice = alcOpenDevice(name.c_str());

			this->CheckCurrentDevice();
		}

		std::string GetCurrentAudioDeviceName() {
			if (this->openALCurrentDevice != nullptr) {
				return alcGetString(this->openALCurrentDevice, ALC_ALL_DEVICES_SPECIFIER);
			} else {
				return "No audio device";
			}
		}

		std::vector<std::string> GetAllAudioDevicesNames() {
		    std::vector<std::string> devices;
		    const ALCchar* deviceList = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);
		    if (deviceList) {
		        while (*deviceList) {
		            devices.push_back(deviceList);
		            deviceList += strlen(deviceList) + 1;
		        }
		    }
		    return devices;
		}

		int GetCurrentAudioDeviceFrequency() {
			ALCint out = 0;
			alcGetIntegerv(this->openALCurrentDevice, ALC_FREQUENCY, 1, &out);
			return out;
		}

		int GetCurrentAudioDeviceMonoSources() {
			ALCint out = 0;
			alcGetIntegerv(this->openALCurrentDevice, ALC_MONO_SOURCES, 1, &out);
			return out;
		}
		
		int GetCurrentAudioDeviceStereoSources() {
			ALCint out = 0;
			alcGetIntegerv(this->openALCurrentDevice, ALC_STEREO_SOURCES, 1, &out);
			return out;
		}
		
		int GetCurrentAudioDeviceRefreshRate() {
			ALint out = 0;
			alcGetIntegerv(this->openALCurrentDevice, ALC_REFRESH, 1, &out);
			return out;
		}
		
		void SetAudioFrequency(int frequency) {
			this->attributes[0] = ALC_FREQUENCY;
			this->attributes[1] = frequency;
		}

		void SetMonoSources(int monoSources) {
			this->attributes[2] = ALC_MONO_SOURCES;
			this->attributes[3] = monoSources;
		}

		void SetStereoSources(int stereoSources) {
			this->attributes[4] = ALC_STEREO_SOURCES;
			this->attributes[5] = stereoSources;
		}

		void SetAudioSync(bool value) {
			this->attributes[6] = ALC_SYNC;
			this->attributes[7] = static_cast<int>(value);
		}

		void SetAudioRefresh(int value) {
			this->attributes[8] = ALC_REFRESH;
			this->attributes[9] = value;
		}

		void LinkAudioSource(AudioSource* audioSource) {
			this->sources.push_back(audioSource);
		}

		void UnLinkAudioSource(AudioSource* audioSource) {
			for(int i = 0; i < this->sources.size(); i++) {
				if (this->sources[i] == audioSource) {
					sources.erase(this->sources.begin() + i);
				}
			}
		}

		AudioListener* GetListener() {
			return &this->listener;
		}

	public:
		ALCcontext* openALContext = nullptr;
		ALCdevice* openALCurrentDevice = nullptr;

		std::vector<AudioSource*> sources;

		AudioListener listener = AudioListener();

	private:
		ALCint attributes[10];
	};
}
