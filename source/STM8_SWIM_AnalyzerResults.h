#ifndef STM8_SWIM_ANALYZER_RESULTS
#define STM8_SWIM_ANALYZER_RESULTS

#include <AnalyzerResults.h>

class STM8_SWIM_Analyzer;
class STM8_SWIM_AnalyzerSettings;

class STM8_SWIM_AnalyzerResults : public AnalyzerResults
{
	void TranslateAnalyzerData(U64 frame_index, std::ostringstream& oss, DisplayBase display_base);
public:
	STM8_SWIM_AnalyzerResults( STM8_SWIM_Analyzer* analyzer, STM8_SWIM_AnalyzerSettings* settings);
	virtual ~STM8_SWIM_AnalyzerResults();

	virtual void GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base );
	virtual void GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id );

	virtual void GenerateFrameTabularText(U64 frame_index, DisplayBase display_base );
	virtual void GeneratePacketTabularText( U64 packet_id, DisplayBase display_base );
	virtual void GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base );

protected: //functions

protected:  //vars
	STM8_SWIM_AnalyzerSettings* mSettings;
	STM8_SWIM_Analyzer* mAnalyzer;
};

#endif //STM8_SWIM_ANALYZER_RESULTS
