#pragma once
#pragma comment(lib, "Ws2_32.lib")

#include <fstream>
#include <memory>

#include "decoded_image.h"
#include "decoder.h"
#include "packet.h"
#include "rtp_receiver.h"
#include "rtp_receiver_listener.h"
#include "settings.h"
#include "y4m_writer.h"

class Receiver : public IRTPPacketListener, public IDecodeCompleteCallback
{
public:
	Receiver();

	void ParseArgs(int argc, const char* argv[]);
	void ValidateArgs();
	void Run();

private:
	// IRTPPacketListener interface
	virtual void OnPacketReceived(RTPPacket InPacket) override;

	// IDecodeCompleteCallback interface
	virtual void OnDecodeComplete(DecodedImage InImage) override;

private:
	std::ostream* OutputStream;
	std::ofstream FileStream;
	Settings	  Options;
	Y4mWriter	  Writer;

	std::shared_ptr<Decoder>	 WrappedDecoder;
	std::shared_ptr<RTPReceiver> RtpReceiver;
};