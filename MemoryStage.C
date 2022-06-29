#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "Memory.h"
#include "MemoryStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"


/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool MemoryStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
	M * mreg = (M *) pregs[MREG];
    	W * wreg = (W *) pregs[WREG];		

	stat = mreg->getstat()->getOutput();
	uint64_t icode = mreg->geticode()->getOutput();
	uint64_t valE = mreg->getvalE()->getOutput();
    	uint64_t dstE = mreg->getdstE()->getOutput();
	dstM = mreg->getdstM()->getOutput();
	uint64_t valA = mreg->getvalA()->getOutput();
	
	bool error = false;
	valM = 0;
	
	uint64_t addr = mem_addr(icode, valA, valE);
	bool memWrite = mem_write(icode);
	bool memRead = mem_read(icode);

	Memory * mem = Memory::getInstance();
	if (memWrite)
		mem->putLong(valA, addr, error);
	if (memRead)
		valM = mem->getLong(addr, error);
	if (error)
		stat = SADR;

	setWInput(wreg, stat, icode, valE, valM, dstE, dstM);
	return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void MemoryStage::doClockHigh(PipeReg ** pregs)
{
	W * wreg = (W *) pregs[WREG];
    	wreg->getstat()->normal();
    	wreg->geticode()->normal();
    	wreg->getvalE()->normal();
    	wreg->getvalM()->normal();
    	wreg->getdstE()->normal();
    	wreg->getdstM()->normal();
}

void MemoryStage::setWInput(W * wreg, uint64_t stat, uint64_t icode, uint64_t ValE, uint64_t ValM, uint64_t dstE, uint64_t dstM)
{
	wreg->getstat()->setInput(stat);
   	wreg->geticode()->setInput(icode);
   	wreg->getvalE()->setInput(ValE);
   	wreg->getvalM()->setInput(ValM);
   	wreg->getdstE()->setInput(dstE);
   	wreg->getdstM()->setInput(dstM);
}

uint64_t MemoryStage::mem_addr(uint64_t icode, uint64_t valA, uint64_t valE)
{
	if (icode == IRMMOVQ || icode == IPUSHQ || icode == ICALL || icode == IMRMOVQ)
		return valE;
	else if (icode == IPOPQ || icode == IRET)
		return valA;
	return 0;
}

bool MemoryStage::mem_read(uint64_t icode)
{
	return (icode == IMRMOVQ || icode == IPOPQ || icode == IRET);
}

bool MemoryStage::mem_write(uint64_t icode)
{
	return (icode == IRMMOVQ || icode == IPUSHQ || icode == ICALL);
}

uint64_t MemoryStage::getvalM()
{
	return valM;
}

uint64_t MemoryStage::getstat()
{
	return stat;
}

uint64_t MemoryStage::getdstM()
{
	return dstM;
}
