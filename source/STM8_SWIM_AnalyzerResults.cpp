#include "STM8_SWIM_AnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "STM8_SWIM_Analyzer.h"
#include "STM8_SWIM_AnalyzerSettings.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

const char* g_swim_cmd[8] = { "SRST", "ROTF", "WOTF", "RSVD", "RSVD" , "RSVD" , "RSVD" , "RSVD" };

STM8_SWIM_AnalyzerResults::STM8_SWIM_AnalyzerResults( STM8_SWIM_Analyzer* analyzer, STM8_SWIM_AnalyzerSettings* settings )
:	AnalyzerResults(),
	mSettings( settings ),
	mAnalyzer( analyzer )
{
}

STM8_SWIM_AnalyzerResults::~STM8_SWIM_AnalyzerResults()
{
}


void STM8_SWIM_AnalyzerResults::TranslateAnalyzerData(U64 frame_index, ostringstream &oss, DisplayBase display_base)
{
	char buffer[128];
	Frame frame = GetFrame(frame_index);

	switch ((swim_bits_state_t)frame.mFlags) {
	case swim_bits_state_t::ACTIVATION:
		oss << "SWIM ACTIVATION";
		break;
	case swim_bits_state_t::SEQUENCE:
		oss << "SWIM ENTRY SEQUENCE";
		break;
	case swim_bits_state_t::SEQUENCE_DONE:
		oss << "SWIM ENTRY SEQUENCE COMPLETED";
		break;
	case swim_bits_state_t::HEADER_CMD: //1 bit
		oss << (frame.mData1 ? "Device" : "Host");
		break;
	case swim_bits_state_t::CMD: //3 bits
		oss << g_swim_cmd[frame.mData1];
		break;
	case swim_bits_state_t::PARITY_CMD: //1 bit
		AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 1, buffer, 128);
		oss << "Parity: " << buffer;
		break;
	case swim_bits_state_t::ACK_CMD: //1 bit
		oss << (frame.mData1 ? "ACK" : "NACK");
		break;
	case swim_bits_state_t::HEADER_DATA: //1 bit
		oss << (frame.mData1 ? "Device" : "Host");
		break;
	case swim_bits_state_t::DATA: //8 bit
		AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, buffer, 128);
		switch ((swim_cmd_state_t)frame.mType) {
		case swim_cmd_state_t::SWIM_SIZE: //8 bit
			oss << "DATA SIZE: " << buffer;
			break;
		case swim_cmd_state_t::SWIM_ADDR: //8 bit
			oss << "DATA ADDR: " << buffer;
			break;
		case swim_cmd_state_t::SWIM_DATA: //8 bit
			oss << "DATA: " << buffer;
			break;
		}
		break;
	case swim_bits_state_t::PARITY_DATA: //1 bit
		AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 1, buffer, 128);
		oss << "Parity: " << buffer;
		break;
	case swim_bits_state_t::ACK_DATA: //1 bit
		oss << (frame.mData1 ? "ACK" : "NACK");
		memcpy(buffer, oss.str().c_str(), oss.str().size() + 1);
		break;
	}
}

void STM8_SWIM_AnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
	char buffer[128];
	ostringstream oss;

	ClearResultStrings();

	TranslateAnalyzerData(frame_index, oss, display_base);

	memcpy(buffer, oss.str().c_str(), oss.str().size() + 1);
	AddResultString(buffer);
}

void STM8_SWIM_AnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
	ostringstream oss;
	std::ofstream file_stream( file, std::ios::out );

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();

	file_stream << "Time [s],Value" << std::endl;

	U64 num_frames = GetNumFrames();
	for( U32 i=0; i < num_frames; i++ )
	{
		Frame frame = GetFrame( i );
		
		char time_str[128];
		AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );

		char number_str[128];
		AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
		TranslateAnalyzerData(i, oss, display_base);

		file_stream << time_str << "," << number_str << oss.str().c_str() << std::endl;

		if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
		{
			file_stream.close();
			return;
		}
	}

	file_stream.close();
}

void STM8_SWIM_AnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
#ifdef SUPPORTS_PROTOCOL_SEARCH
	char buffer[128];
	ostringstream oss;

	ClearTabularText();

	TranslateAnalyzerData(frame_index, oss, display_base);

	memcpy(buffer, oss.str().c_str(), oss.str().size() + 1);
	AddTabularText( buffer );
#endif
}

void STM8_SWIM_AnalyzerResults::GeneratePacketTabularText( U64 packet_id, DisplayBase display_base )
{
	//not supported

}

void STM8_SWIM_AnalyzerResults::GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base )
{
	//not supported
}