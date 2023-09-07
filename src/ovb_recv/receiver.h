#pragma once
#pragma comment(lib, "Ws2_32.lib")

#include <fstream>
#include <memory>
#include <string>

#include "ovb_common/image/decoded_image.h"
#include "ovb_recv/decoders/decoder.h"
#include "ovb_recv/depacketizer/depacketizer.h"
#include "ovb_recv/depacketizer/depacketizer_listener.h"
#include "ovb_common/rtp/packet.h"
#include "ovb_recv/rtp_receiver/rtp_receiver.h"
#include "ovb_recv/rtp_receiver/rtp_receiver_listener.h"
#include "ovb_recv/vmaf/vmaf.h"
#include "ovb_common/settings.h"
#include "ovb_common/y4m/y4m_writer.h"

class ReceiverSettings : public Settings
{
public:
	std::string ModelPath;
};

class Receiver : public IRTPPacketListener, public IDecodeCompleteCallback, public IDepacketizerListener
{
public:
	Receiver();
	~Receiver();

	void ParseArgs(int argc, const char* argv[]);
	void ValidateArgs();
	void PrintSettings();
	void Run();

private:
	void PrintHelp();

	// IRTPPacketListener interface
	virtual void OnPacketReceived(RTPPacket InPacket) override;

	// IDecodeCompleteCallback interface
	virtual void OnDecodeComplete(DecodedImage InImage) override;

	// IDepacketizerListener interface
	virtual void OnNALReceived(uint8_t* InData, size_t InSize) override;

private:
	std::ostream*	 OutputStream;
	std::ofstream	 FileStream;
	ReceiverSettings Options;
	Y4mWriter		 Writer;
	DecoderConfig	 Config;

	VmafContext*				  Vmaf;
	std::shared_ptr<Decoder>	  WrappedDecoder;
	std::shared_ptr<RTPReceiver>  RtpReceiver;
	std::shared_ptr<Depacketizer> Depacketizer;

private:
	VMAF_API_FUNCTION_LIST VmafApiPtrs;
};