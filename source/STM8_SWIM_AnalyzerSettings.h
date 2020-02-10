#ifndef STM8_SWIM_ANALYZER_SETTINGS
#define STM8_SWIM_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class STM8_SWIM_AnalyzerSettings : public AnalyzerSettings
{
public:
	STM8_SWIM_AnalyzerSettings();
	virtual ~STM8_SWIM_AnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	
	Channel mInputChannel;
	U32 mBitRate;

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mBitRateInterface;
};

#endif //STM8_SWIM_ANALYZER_SETTINGS
