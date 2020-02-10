#include "STM8_SWIM_AnalyzerSettings.h"
#include <AnalyzerHelpers.h>


STM8_SWIM_AnalyzerSettings::STM8_SWIM_AnalyzerSettings()
:	mInputChannel( UNDEFINED_CHANNEL ),
	mBitRate( 0 )
{
	mInputChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mInputChannelInterface->SetTitleAndTooltip( "STM8 SWIM", "STM8 SWIM" );
	mInputChannelInterface->SetChannel( mInputChannel );

	mBitRateInterface.reset( new AnalyzerSettingInterfaceInteger() );

	AddInterface( mInputChannelInterface.get() );

	AddExportOption( 0, "Export as text/csv file" );
	AddExportExtension( 0, "text", "txt" );
	AddExportExtension( 0, "csv", "csv" );

	ClearChannels();
	AddChannel( mInputChannel, "STM8 SWIM", false );
}

STM8_SWIM_AnalyzerSettings::~STM8_SWIM_AnalyzerSettings()
{
}

bool STM8_SWIM_AnalyzerSettings::SetSettingsFromInterfaces()
{
	mInputChannel = mInputChannelInterface->GetChannel();
	mBitRate = mBitRateInterface->GetInteger();

	ClearChannels();
	AddChannel( mInputChannel, "STM8 SWIM", true );

	return true;
}

void STM8_SWIM_AnalyzerSettings::UpdateInterfacesFromSettings()
{
	mInputChannelInterface->SetChannel( mInputChannel );
	mBitRateInterface->SetInteger( mBitRate );
}

void STM8_SWIM_AnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	text_archive >> mInputChannel;

	ClearChannels();
	AddChannel( mInputChannel, "STM8 SWIM", true );

	UpdateInterfacesFromSettings();
}

const char* STM8_SWIM_AnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << mInputChannel;

	return SetReturnString( text_archive.GetString() );
}
