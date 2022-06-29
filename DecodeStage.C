#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "E.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "DecodeStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "Tools.h"
#include "ExecuteStage.h"
#include "MemoryStage.h"

/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool DecodeStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
	D * dreg = (D *) pregs[DREG];
        E * ereg = (E *) pregs[EREG];
	
	ExecuteStage * es = (ExecuteStage*) stages[ESTAGE];
	RegisterFile * rf = RegisterFile::getInstance();	
	bool error = false;

        uint64_t stat = dreg->getstat()->getOutput();
        uint64_t icode = dreg->geticode()->getOutput();
        uint64_t ifun = dreg->getifun()->getOutput();
        uint64_t valC = dreg->getvalC()->getOutput();
	uint64_t rA = dreg->getrA()->getOutput();
	uint64_t rB = dreg->getrB()->getOutput();
	uint64_t valP = dreg->getvalP()->getOutput();
	
	srcA = d_srcA(rA, icode);
        srcB = d_srcB(rB, icode);
	uint64_t dstE = d_dstE(rB, icode);
        uint64_t dstM = d_dstM(rA, icode);

	uint64_t eicode = ereg->geticode()->getOutput();
        uint64_t edstm = ereg->getdstM()->getOutput();
	uint64_t ecnd = es->get_ecnd();
	ebubble = calculateControlSignals(eicode, edstm, srcA, srcB, ecnd);

	uint64_t rvalA = rf->readRegister(srcA, error);
	uint64_t rvalB = rf->readRegister(srcB, error);
        uint64_t valA = d_valA(pregs, stages, icode, srcA, rvalA, valP);
        uint64_t valB = d_valB(pregs, stages, icode, srcB, rvalB);

        setEInput(ereg, stat, icode, ifun, valC, valA, valB, dstE, dstM, srcA, srcB);
	return false;   
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void DecodeStage::doClockHigh(PipeReg ** pregs)
{
	if (ebubble)
		bubble(pregs);
	else
		normal(pregs);
}

void DecodeStage::normal(PipeReg ** pregs)
{
	E * ereg = (E *) pregs[EREG];
        ereg->getstat()->normal();
        ereg->geticode()->normal();
        ereg->getifun()->normal();
        ereg->getvalC()->normal();
        ereg->getvalA()->normal();
        ereg->getvalB()->normal();
        ereg->getdstE()->normal();
        ereg->getdstM()->normal();
        ereg->getsrcA()->normal();
        ereg->getsrcB()->normal();
}

void DecodeStage::bubble(PipeReg ** pregs)
{
	E * ereg = (E *) pregs[EREG];
        ereg->getstat()->bubble(SAOK);
        ereg->geticode()->bubble(INOP);
        ereg->getifun()->bubble();
        ereg->getvalC()->bubble();
        ereg->getvalA()->bubble();
        ereg->getvalB()->bubble();
        ereg->getdstE()->bubble(RNONE);
        ereg->getdstM()->bubble(RNONE);
        ereg->getsrcA()->bubble(RNONE);
        ereg->getsrcB()->bubble(RNONE);
}

void DecodeStage::setEInput(E * ereg, uint64_t stat, uint64_t icode, uint64_t ifun, uint64_t ValC, uint64_t ValA, uint64_t ValB, uint64_t dstE, uint64_t dstM, uint64_t srcA, uint64_t srcB)
{
	ereg->getstat()->setInput(stat);
   	ereg->geticode()->setInput(icode);
   	ereg->getifun()->setInput(ifun);
   	ereg->getvalC()->setInput(ValC);
   	ereg->getvalA()->setInput(ValA);
   	ereg->getvalB()->setInput(ValB);
   	ereg->getdstE()->setInput(dstE);
   	ereg->getdstM()->setInput(dstM);
   	ereg->getsrcA()->setInput(srcA);
   	ereg->getsrcB()->setInput(srcB);
}

uint64_t DecodeStage::d_srcA(uint64_t rA, uint64_t icode)
{
	if (icode == IRRMOVQ || icode == IRMMOVQ || icode == IOPQ || icode == IPUSHQ)
		return rA;
	else if (icode == IPOPQ || icode == IRET)
		return RSP;
	return RNONE;
}

uint64_t DecodeStage::d_srcB(uint64_t rB, uint64_t icode)
{
	if (icode == IOPQ || icode == IRMMOVQ || icode == IMRMOVQ) 
		return rB;
	else if (icode == IPUSHQ || icode == IPOPQ || icode == ICALL || icode == IRET)
		return RSP;
	return RNONE;
}

uint64_t DecodeStage::d_dstE(uint64_t rB, uint64_t icode)
{
	if (icode == IRRMOVQ || icode == IIRMOVQ || icode == IOPQ)
		return rB;
	else if (icode == IPUSHQ || icode == IPOPQ || icode == ICALL || icode == IRET)
		return RSP;
	return RNONE;
}

uint64_t DecodeStage::d_dstM(uint64_t rA, uint64_t icode)
{
	if (icode == IMRMOVQ || icode == IPOPQ)
		return rA;
	return RNONE;
}

uint64_t DecodeStage::d_valA(PipeReg ** pregs, Stage ** stages, uint64_t icode, uint64_t srcA, uint64_t rvalA, uint64_t valP)
{
	M * mreg = (M *) pregs[MREG];
	W * wreg = (W *) pregs[WREG];
	ExecuteStage * estage = (ExecuteStage *) stages[ESTAGE];
	bool error = false;
	MemoryStage * mstage = (MemoryStage *) stages[MSTAGE];

	if (icode == ICALL || icode == IJXX) return valP;
	if (srcA == RNONE) return 0;
	if (srcA == estage->getdstE()) return estage->getvalE();
	if (srcA == mreg->getdstM()->getOutput()) return mstage->getvalM();
	if (srcA == mreg->getdstE()->getOutput()) return mreg->getvalE()->getOutput();
	if (srcA == wreg->getdstM()->getOutput()) return wreg->getvalM()->getOutput();
	if (srcA == wreg->getdstE()->getOutput()) return wreg->getvalE()->getOutput();
	return rvalA;
}

uint64_t DecodeStage::d_valB(PipeReg ** pregs, Stage ** stages, uint64_t icode, uint64_t srcB, uint64_t rvalB)
{
	M * mreg = (M *) pregs[MREG];
        W * wreg = (W *) pregs[WREG];
	MemoryStage * mstage = (MemoryStage *) stages[MSTAGE];
        ExecuteStage * estage = (ExecuteStage *) stages[ESTAGE];
        bool error = false;

	if (srcB == RNONE) return 0;
	if (srcB == estage->getdstE()) return estage->getvalE();
	if (srcB == mreg->getdstM()->getOutput()) return mstage->getvalM();
	if (srcB == mreg->getdstE()->getOutput()) return mreg->getvalE()->getOutput();
	if (srcB == wreg->getdstM()->getOutput()) return wreg->getvalM()->getOutput();
	if (srcB == wreg->getdstE()->getOutput()) return wreg->getvalE()->getOutput();	
        return rvalB;
}

uint64_t DecodeStage::getdsrcA()
{
	return srcA;
}

uint64_t DecodeStage::getdsrcB()
{
	return srcB;
}

bool DecodeStage::calculateControlSignals(uint64_t eicode, uint64_t edstm, uint64_t dsrca, uint64_t dsrcb, uint64_t cnd)
{
	return (eicode == IJXX && !cnd) || (eicode == IMRMOVQ || eicode == IPOPQ) && 
		(edstm == dsrca || edstm == dsrcb);
}
