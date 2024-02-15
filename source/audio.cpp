#include <PrettyEngine/audio.hpp>

#include <fstream>

namespace PrettyEngine {
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

	void AudioSource::LoadFrequency(float frequency, float duration, float sampleRate) {
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

	void AudioSource::LoadWAVAudio(std::string fileName) {
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

}