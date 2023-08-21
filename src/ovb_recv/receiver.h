#pragma once
#pragma comment(lib, "Ws2_32.lib")

#include <fstream>
#include <memory>

#include "ovb_recv/image/decoded_image.h"
#include "ovb_recv/decoders/decoder.h"
#include "ovb_recv/depacketizer/depacketizer.h"
#include "ovb_recv/depacketizer/depacketizer_listener.h"
#include "ovb_common/rtp/packet.h"
#include "ovb_recv/rtp_receiver/rtp_receiver.h"
#include "ovb_recv/rtp_receiver/rtp_receiver_listener.h"
#include "ovb_common/settings.h"
#include "ovb_recv/y4m_writer.h"

class Receiver : public IRTPPacketListener, public IDecodeCompleteCallback, public IDepacketizerListener
{
public:
	Receiver();

	void ParseArgs(int argc, const char* argv[]);
	void ValidateArgs();
	void Run();

private:
	void PrintHelp();

	// IRTPPacketListener interface
	virtual void OnPacketReceived(RTPPacket InPacket) override;

	// IDecodeCompleteCallback interface
	virtual void OnDecodeComplete(DecodedImage InImage) override;

	// IDepacketizerListener interface
	virtual void OnNALReceived(const uint8_t* InData, size_t InSize) override;

private:
	std::ostream* OutputStream;
	std::ofstream FileStream;
	Settings	  Options;
	Y4mWriter	  Writer;

	std::shared_ptr<Decoder>	  WrappedDecoder;
	std::shared_ptr<RTPReceiver>  RtpReceiver;
	std::shared_ptr<Depacketizer> Depacketizer;
};