/// @file

#ifndef __RDA5807_HPP
#define __RDA5807_HPP

#include "Radio.hpp"
#include "radioDataSystem.hpp"

/// \brief
/// RDA5807 Interface
/// \details
/// This is an interface that simplifies the use of RDA5807 chips. Tuning, Muting, Auto-search
/// and Setting of Frequencies are some of its posibilites. It is compatible with all RDA58XX chips,
/// though some RDA58XX chips have no registers for RDS-Data thus not having RDS-abilities.
class RDA5807 : public Radio {
	private:
		//I2C Communication
		uint16_t data[8] = {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x9881, 0x0000, 0x8000};	//First two bytes contain device specific info; are never send but here for completeness.
		uint16_t status[6] = {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};
		const uint8_t indexAddress;			//0x11 for RDA5807
		const uint8_t firstReadRegister;	//0x10 for RDA5807
		void setData() override;
		void setData(const unsigned int regNumber);
		void setRegister(const unsigned int regNumber, const uint16_t value);

		void getStatus() override;
		void getStatus(const unsigned int regNumber);

		//Specific Powerfull Setting; let user handle this through standBy().
		void powerUpEnable(const bool enable);
	public:
		RDA5807(hwlib::i2c_bus_bit_banged_scl_sda & bus, const uint8_t address = 0x10, const uint8_t firstReadRegister = 0x0A, const int bandLimit = 0);

		//Basic Settings
		void setClockSupply(const bool clockSource, const bool directInput = false);
		void setClockFrequency(const unsigned int frequency = 32) override;
		unsigned int getClockFrequency() override;

		void begin(const bool muted = false);
		void reset();

		void setBandLimit(const unsigned int limit = 0) override;
		unsigned int hasBandLimit() override;

		void setSpacing(const unsigned int spacing = 100000);		//Spacing between channels; 100000kHz default (Country Specific).
		unsigned int getSpacing();

		void standBy(const bool sleep = true) override;
		bool isStandBy() override;

		//Basic Tuning
		bool setFrequency(const float frequency, const bool autoTune);
		void setFrequency(const float frequency) override;
		float getFrequency() override;
		unsigned int getIntFrequency() override;

		void setTune(const bool tune = true);
		bool isTuned();

		void demodulateMethod(const bool newMethod = true);		//Can Improve Signal Strength
		bool newDemodulate();

		bool isReady();
		bool isStation();

		//Audio Quality
		unsigned int signalStrength() override;

		void normalAudio(const bool normal = true);		//High Impedance (false) or Normal (true) Output
		bool isNormalAudio();

		void setBassBoost(const bool boost = true);
		bool bassBoosted();

		void setVolume(const uint8_t volume = 15);
		unsigned int getVolume();

		void setMute(const bool mute = true) override;
		bool isMuted(const char side = 'a') override;

		void setStereo(const bool stereo = true) override;
		bool isStereo() override;
		bool stereoReception() override;

		//Search Settings
		void seek(const unsigned int direction) override;

		void seekChannel(const unsigned int direction, const bool wrapContinue = true);
		bool seekCompleted();
		void setSeekThreshold(const uint8_t threshold = 8);

		//Radio Data System
		radioDataSystem radioData;

		void enableRadioData(const bool enable = true);
		bool radioDataEnabled();

		bool radioDataReady();
		bool radioDataSynced();
		unsigned int radioDataErrors(const unsigned int block);

		//Some often used functions are available as method others can be called from radioData (e.g. radioData.getProgramType())
		char* getStationName(const unsigned int dataValidity = 4);
		char* stationName();
		char* getStationText();
		char* stationText();

		//Raw Radio Data System
		void printRawRadioData();
		void updateRadioData();
};

#endif //__RDA5807_HPP