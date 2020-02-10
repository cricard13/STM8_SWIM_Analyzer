#ifndef STM8_SWIM_SIMULATION_DATA_GENERATOR
#define STM8_SWIM_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
#include <string>
class STM8_SWIM_AnalyzerSettings;

class STM8_SWIM_SimulationDataGenerator
{
public:
	STM8_SWIM_SimulationDataGenerator();
	~STM8_SWIM_SimulationDataGenerator();

	void Initialize( U32 simulation_sample_rate, STM8_SWIM_AnalyzerSettings* settings );
	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel );

protected:
	STM8_SWIM_AnalyzerSettings* mSettings;
	U32 mSimulationSampleRateHz;

protected:
	void CreateSerialByte();
	std::string mSerialText;
	U32 mStringIndex;

	SimulationChannelDescriptor mSerialSimulationData;

};
#endif //STM8_SWIM_SIMULATION_DATA_GENERATOR