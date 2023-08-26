#include "ovb_common/common.h"

#include "ovb_relay/drop/dropper_factory.h"
#include "ovb_relay/drop/simple_bursty/simple_bursty_dropper.h"
#include "ovb_relay/drop/complex_bursty/complex_bursty_dropper.h"
#include "ovb_relay/drop/continuous/continuous_dropper.h"

std::shared_ptr<Dropper> DropperFactory::Create(EDropType InDropType, DropSettings InOptions)
{
	switch (InDropType)
	{
		case EDropType::LOSS_SIMPLE_BURSTY:
			return std::make_shared<SimpleBurstyDropper>(InOptions);
		case EDropType::LOSS_COMPLEX_BURSTY:
			return std::make_shared<ComplexBurstyDropper>(InOptions);
		case EDropType::LOSS_CONTINUOUS:
			return std::make_shared<ContinuousDropper>(InOptions);
		default:
			unimplemented();
			return nullptr;
	}
}