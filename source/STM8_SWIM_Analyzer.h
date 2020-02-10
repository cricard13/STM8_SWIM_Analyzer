#ifndef STM8_SWIM_ANALYZER_H
#define STM8_SWIM_ANALYZER_H

#include <Analyzer.h>
#include "STM8_SWIM_AnalyzerResults.h"
#include "STM8_SWIM_SimulationDataGenerator.h"

typedef enum class _state {
	ACTIVATION,
	SEQUENCE,
	SEQUENCE_DONE,
	HEADER_CMD,
	CMD,
	PARITY_CMD,
	ACK_CMD,
	HEADER_DATA,
	DATA,
	PARITY_DATA,
	ACK_DATA,
} swim_bits_state_t;

typedef enum class _swim_cmd_state {
	SWIM_NODATA,
	SWIM_SIZE,
	SWIM_ADDR,
	SWIM_DATA,
} swim_cmd_state_t;

class STM8_SWIM_AnalyzerSettings;
class ANALYZER_EXPORT STM8_SWIM_Analyzer : public Analyzer2
{
public:
	STM8_SWIM_Analyzer();
	virtual ~STM8_SWIM_Analyzer();

	virtual void SetupResults();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

	bool DecodeData(void);

protected: //vars
	std::auto_ptr< STM8_SWIM_AnalyzerSettings > mSettings;
	std::auto_ptr< STM8_SWIM_AnalyzerResults > mResults;
	AnalyzerChannelData* mSerial;

	STM8_SWIM_SimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitilized;

	//Serial analysis vars:
	U32 mSampleRateHz;
	U32 mStartOfStopBitOffset;
	U32 mEndOfStopBitOffset;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //STM8_SWIM_ANALYZER_H
