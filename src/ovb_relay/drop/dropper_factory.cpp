#include "ovb_common/common.h"

#include "ovb_relay/drop/dropper_factory.h"
#include "ovb_relay/drop/bursty/bursty_dropper.h"
#include "ovb_relay/drop/continuous/continuous_dropper.h"

std::shared_ptr<Dropper> DropperFactory::Create(EDropType InDropType, DropSettings InOptions)
{
	switch (InDropType)
	{
		case EDropType::LOSS_BURSTY:
			return std::make_shared<BurstyDropper>(InOptions);
		case EDropType::LOSS_CONTINUOUS:
			return std::make_shared<ContinuousDropper>(InOptions);
		default:
			unimplemented();
			return nullptr;
	}
}