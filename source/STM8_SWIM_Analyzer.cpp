#include "STM8_SWIM_Analyzer.h"
#include "STM8_SWIM_AnalyzerSettings.h"
#include <AnalyzerChannelData.h>

static U32 g_expected_num_bits[11] = { 1, // ACTIVATION
									   8, // SEQUENCE
									   1, // SEQUENCE DONE
									   1, // HEADER_CMD
									   3, // CMD
									   1, // PARITY_CMD
									   1, // ACK_CMD
									   1, // HEADER_DATA
									   8, // DATA
									   1, // PARITY_DATA
									   1  // ACK_DATA
									 };

/* 16uS */
#define MAXIMUM_LOW_PERIOD_VALID_DATA_S 0.000016

STM8_SWIM_Analyzer::STM8_SWIM_Analyzer()
:	Analyzer2(),  
	mSettings( new STM8_SWIM_AnalyzerSettings() ),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( mSettings.get() );
}

STM8_SWIM_Analyzer::~STM8_SWIM_Analyzer()
{
	KillThread();
}

void STM8_SWIM_Analyzer::SetupResults()
{
	mResults.reset( new STM8_SWIM_AnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );
}


bool STM8_SWIM_Analyzer::DecodeData(void)
{
	U8 data;
	U8 mask;
	Frame frame;
	U64 starting_sample;
	U32 i;
	BitState state = mSerial->GetBitState();	
	U64 low_pulse_width = 0, high_pulse_width = 0, 
		low_pulse_reference = 0, low_pulse_activation = 0;
	swim_bits_state_t bit_state = swim_bits_state_t::ACTIVATION;
	swim_cmd_state_t cmd_state;
	U8 cmd, data_size, size;

	mSampleRateHz = GetSampleRate();

	//The bit format is a return-to-zero format.
	//Let's move to the next edge if the line is currently high.
	if (state == BIT_HIGH) {
		mSerial->AdvanceToNextEdge();
	}

	//Compute a low pulse reference to filter out specific sequence or bad data
	low_pulse_reference = (U64)ceil((double)MAXIMUM_LOW_PERIOD_VALID_DATA_S / (double)mSampleRateHz);

	for (; ; ) {
		data = 0;
		//Compute the mask relative to the sequence
		mask = 1 << (g_expected_num_bits[(U8)bit_state] - 1);

		//Move to next sample
		starting_sample = mSerial->GetSampleNumber();
		for (i = 0; i < g_expected_num_bits[(U8)bit_state]; i++)
		{
			//We are now checking the duration of a low period vs high period
			//to determine if we have a bit '1' or '0'
			mSerial->TrackMinimumPulseWidth();

			state = mSerial->GetBitState(); //LOW
			mSerial->AdvanceToNextEdge();
			state = mSerial->GetBitState(); //HIGH
			low_pulse_width = mSerial->GetMinimumPulseWidthSoFar();
			
			mSerial->TrackMinimumPulseWidth();
			mSerial->AdvanceToNextEdge(); //falling edge -- beginning of the start bit
			high_pulse_width = mSerial->GetMinimumPulseWidthSoFar();

			//let's put a dot exactly where we sample this bit:
			mResults->AddMarker(mSerial->GetSampleNumber(), AnalyzerResults::Dot, mSettings->mInputChannel);
			
			//Manage some exceptions in the SWIM protocol
			//Check if we met a SWIM ACTIVATION sequence.
			if (((double)low_pulse_width / (double)mSampleRateHz) * 1000 > 1) {
				bit_state = swim_bits_state_t::ACTIVATION;
				low_pulse_activation = low_pulse_width;
			}
			//Check if we met glitches to discard during the SWIM activation sequence probing.
			else if (bit_state == swim_bits_state_t::SEQUENCE && ((double)low_pulse_width < (double)low_pulse_activation / (double)4)) {
				--i;
			}
			//If the SWIM activation sequence is missing consider this bit as a header for a command.
			else if (bit_state == swim_bits_state_t::ACTIVATION && ((double)low_pulse_width/(double)mSampleRateHz) * 1000 < 1) {
				bit_state = swim_bits_state_t::HEADER_CMD;
			}
			//Discard glitches
			else if (bit_state == swim_bits_state_t::HEADER_CMD && low_pulse_width > low_pulse_reference) {
				break;
			}
			else {
				//High pulse duration is greater than a low pulse duration then we consider as a bit '1'
				if (high_pulse_width > low_pulse_width)
					data |= mask;

				mask = mask >> 1;
			}
		}

		//Manage the state machine to decode the flow
		if (i == g_expected_num_bits[(U8)bit_state]) {
			frame.mData1 = data;
			switch (bit_state) {
			case swim_bits_state_t::ACTIVATION:
				frame.mFlags = (U8)swim_bits_state_t::ACTIVATION;
				bit_state = swim_bits_state_t::SEQUENCE;
				break;
			case swim_bits_state_t::SEQUENCE:
				frame.mFlags = (U8)swim_bits_state_t::SEQUENCE;
				bit_state = swim_bits_state_t::SEQUENCE_DONE;
				break;
			case swim_bits_state_t::SEQUENCE_DONE:
				frame.mFlags = (U8)swim_bits_state_t::SEQUENCE_DONE;
				bit_state = swim_bits_state_t::HEADER_CMD;
				low_pulse_reference = low_pulse_width - 1;
				break;
			case swim_bits_state_t::HEADER_CMD: //1 bit
				frame.mFlags = (U8)swim_bits_state_t::HEADER_CMD;
				bit_state = swim_bits_state_t::CMD;
				break;
			case swim_bits_state_t::CMD: //3 bits
				frame.mFlags = (U8)swim_bits_state_t::CMD;
				bit_state = swim_bits_state_t::PARITY_CMD;
				cmd = data;
				break;
			case swim_bits_state_t::PARITY_CMD: //1 bit
				frame.mFlags = (U8)swim_bits_state_t::PARITY_CMD;
				bit_state = swim_bits_state_t::ACK_CMD;
				if (data == 0) {
					cmd_state = swim_cmd_state_t::SWIM_NODATA;
				}
				else {
					cmd_state = swim_cmd_state_t::SWIM_SIZE;
				}				
				break;
			case swim_bits_state_t::ACK_CMD: //1 bit
				frame.mFlags = (U8)swim_bits_state_t::ACK_CMD;
				if (cmd_state == swim_cmd_state_t::SWIM_SIZE) {
					bit_state = swim_bits_state_t::HEADER_DATA;
				}
				else {
					bit_state = swim_bits_state_t::HEADER_CMD;
				}

				break;
			case swim_bits_state_t::HEADER_DATA: //1 bit
				frame.mFlags = (U8)swim_bits_state_t::HEADER_DATA;
				bit_state = swim_bits_state_t::DATA;
				break;
			case swim_bits_state_t::DATA: //8 bit
				frame.mFlags = (U8)swim_bits_state_t::DATA;
				bit_state = swim_bits_state_t::PARITY_DATA;
				switch (cmd_state) {
				case swim_cmd_state_t::SWIM_SIZE:
					frame.mType = (U8)swim_cmd_state_t::SWIM_SIZE;
					data_size = data;
					size = 3;
					cmd_state = swim_cmd_state_t::SWIM_ADDR;
					break;
				case swim_cmd_state_t::SWIM_ADDR:
					frame.mType = (U8)swim_cmd_state_t::SWIM_ADDR;
					--size;
					if (size == 0) {
						size = data_size;
						cmd_state = swim_cmd_state_t::SWIM_DATA;
					}
					break;
				case swim_cmd_state_t::SWIM_DATA:
					frame.mType = (U8)swim_cmd_state_t::SWIM_DATA;
					--size;
					break;
				}
				break;
			case swim_bits_state_t::PARITY_DATA: //1 bit
				frame.mFlags = (U8)swim_bits_state_t::PARITY_DATA;
				bit_state = swim_bits_state_t::ACK_DATA;
				break;
			case swim_bits_state_t::ACK_DATA: //1 bit
				frame.mFlags = (U8)swim_bits_state_t::ACK_DATA;

				if (size > 0) {
					bit_state = swim_bits_state_t::HEADER_DATA;
				}
				else {
					bit_state = swim_bits_state_t::HEADER_CMD;
				}
				break;
			}

			frame.mStartingSampleInclusive = starting_sample;
			frame.mEndingSampleInclusive = mSerial->GetSampleNumber();

			mResults->AddFrame(frame);
			mResults->CommitResults();
			ReportProgress(frame.mEndingSampleInclusive);
		}
	}

	return true;
}

void STM8_SWIM_Analyzer::WorkerThread()
{
	mSerial = GetAnalyzerChannelData( mSettings->mInputChannel );
	for( ; ; )
	{
		DecodeData();
	}
}

bool STM8_SWIM_Analyzer::NeedsRerun()
{
	return false;
}

U32 STM8_SWIM_Analyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 STM8_SWIM_Analyzer::GetMinimumSampleRateHz()
{
	return mSettings->mBitRate * 4;
}

const char* STM8_SWIM_Analyzer::GetAnalyzerName() const
{
	return "STM8 SWIM";
}

const char* GetAnalyzerName()
{
	return "STM8 SWIM";
}

Analyzer* CreateAnalyzer()
{
	return new STM8_SWIM_Analyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}