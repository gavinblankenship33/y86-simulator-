#include <string>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "E.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "ConditionCodes.h"
#include "ExecuteStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "Tools.h"
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
bool ExecuteStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
	E * ereg = (E *) pregs[EREG];
    	M * mreg = (M *) pregs[MREG];
	W * wreg = (W *) pregs[WREG];	
	MemoryStage * mstage = (MemoryStage *) stages[MSTAGE];

	uint64_t stat = ereg->getstat()->getOutput();
    	uint64_t icode = ereg->geticode()->getOutput();
	uint64_t valA = ereg->getvalA()->getOutput();
	dstE = ereg->getdstE()->getOutput();	
	uint64_t dstM = ereg->getdstM()->getOutput();
	uint64_t ifun = ereg->getifun()->getOutput();

	cnd = 0;
	valE = 0;

	uint64_t valB = ereg->getvalB()->getOutput();
	uint64_t valC = ereg->getvalC()->getOutput();
    	
	uint64_t alufunResult = alufun(icode, ifun);
        uint64_t aluAResult = aluA(icode, valA, valC);
        uint64_t aluBResult = aluB(icode, valB);
	uint64_t mStat = mstage->getstat();
        uint64_t wStat = wreg->getstat()->getOutput();
	bool set = set_cc(ereg, mStat, wStat);
	valE = alu(alufunResult, aluAResult, aluBResult, set);
	
	mbubble = calculateControlSignals(mStat, wStat);
	cnd = cc(icode, ifun);
        dstE = e_dstE(icode, dstE, cnd);

	setMInput(mreg, stat, icode, cnd, valE, valA, dstE, dstM);
	return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void ExecuteStage::doClockHigh(PipeReg ** pregs)
{
	M * mreg = (M *) pregs[MREG];
	
	if(mbubble)
		bubble(pregs);
	else
		normal(pregs);
}

void ExecuteStage::normal(PipeReg ** pregs)
{
	M * mreg = (M *) pregs[MREG];
	mreg->getstat()->normal();
        mreg->geticode()->normal();
        mreg->getCnd()->normal();
        mreg->getvalE()->normal();
	mreg->getvalA()->normal();
	mreg->getdstE()->normal();
	mreg->getdstM()->normal();
}

void ExecuteStage::bubble(PipeReg ** pregs)
{
	M * mreg = (M *) pregs[MREG];
	mreg->getstat()->bubble(SAOK);
	mreg->geticode()->bubble(INOP);
	mreg->getCnd()->bubble();
	mreg->getvalE()->bubble();
	mreg->getvalA()->bubble();
 	mreg->getdstE()->bubble(RNONE);	
	mreg->getdstM()->bubble(RNONE);
}
 
void ExecuteStage::setMInput(M * mreg, uint64_t stat, uint64_t icode, uint64_t cnd, uint64_t ValE, uint64_t ValA, uint64_t dstE, uint64_t dstM)
{
   	mreg->getstat()->setInput(stat);
    	mreg->geticode()->setInput(icode);
    	mreg->getCnd()->setInput(cnd);
    	mreg->getvalE()->setInput(ValE);
    	mreg->getvalA()->setInput(ValA);
    	mreg->getdstE()->setInput(dstE);
	mreg->getdstM()->setInput(dstM);
}

uint64_t ExecuteStage::aluA(uint64_t icode, uint64_t valA, uint64_t valC)
{
	if (icode == IRRMOVQ || icode == IOPQ)
		return valA;
	else if (icode == IIRMOVQ || icode == IRMMOVQ || icode == IMRMOVQ)
		return valC;
	else if (icode == ICALL || icode == IPUSHQ)
		return -8;
	else if (icode == IRET || icode == IPOPQ)
		return 8;

	return 0;
}

uint64_t ExecuteStage::aluB(uint64_t icode, uint64_t valB)
{
	if (icode == IRMMOVQ || icode == IMRMOVQ || icode == IOPQ || 
	icode == ICALL || icode == IPUSHQ || icode == IRET || 
	icode == IPOPQ)
		return valB;
	else if (icode == IRRMOVQ || icode == IIRMOVQ)
		return 0;

	return 0;
}

uint64_t ExecuteStage::alufun(uint64_t icode, uint64_t ifun)
{
	if (icode == IOPQ) return ifun;
	return ADDQ;
}

bool ExecuteStage::set_cc(E * ereg, uint64_t m_stat, uint64_t w_stat)
{
	return (ereg->geticode()->getOutput() == IOPQ) && 
	(m_stat != SADR && m_stat != SINS && m_stat != SHLT) && 
	(w_stat != SADR && w_stat != SINS && w_stat != SHLT);
}

uint64_t ExecuteStage::e_dstE(uint64_t icode, uint64_t dstE, uint64_t cnd)
{
	if (icode == IRRMOVQ && !cnd)
		return RNONE;
	return dstE;
}

uint64_t ExecuteStage::alu(uint64_t alufunResult, uint64_t aluAResult, uint64_t aluBResult, bool setCC)
{	
	uint64_t result = -1;
	bool overflow = false;
	if (alufunResult == ADDQ)
	{
		overflow = Tools::addOverflow(aluAResult, aluBResult);
                result = aluAResult + aluBResult;
	}
	else if (alufunResult == ANDQ)
		result = aluAResult & aluBResult;
	else if (alufunResult == XORQ)
		result = aluAResult ^ aluBResult;
	else if (alufunResult == SUBQ)
	{
		overflow = Tools::subOverflow(aluAResult, aluBResult);
                result = aluBResult - aluAResult;
	}

	if (setCC)
	{
		ConditionCodes * condC = ConditionCodes::getInstance();
		bool error = false;
		
		uint64_t ZFFlag = (result == 0 ? 1 : 0);
		condC->setConditionCode(ZFFlag, ZF, error);
		condC->setConditionCode(Tools::sign(result), SF, error);
		condC->setConditionCode(overflow, OF, error);
	}
	return result;
}


uint64_t ExecuteStage::cc(uint64_t icode, uint64_t ifun)
{
	bool error = false;
	ConditionCodes * cc = ConditionCodes::getInstance();
	bool sf = cc->getConditionCode(SF, error);
	bool of = cc->getConditionCode(OF, error);
	bool zf = cc->getConditionCode(ZF, error);

	if (icode == IJXX || icode == ICMOVXX)
	{
		if (ifun == UNCOND) return 1;
		if (ifun == LESSEQ) return (sf ^ of) || zf;
		if (ifun == LESS) return (sf ^ of);
		if (ifun == EQUAL) return zf;
		if (ifun == NOTEQUAL) return !zf;
		if (ifun == GREATER) return !(sf ^ of) && !zf;
		if (ifun == GREATEREQ) return !(sf ^ of);
		return 0;
	}
	return 0;
}

uint64_t ExecuteStage::getdstE()
{
	return dstE;
}

uint64_t ExecuteStage::getvalE()
{
	return valE;
}

uint64_t ExecuteStage::get_ecnd()
{
	return cnd;
}

bool ExecuteStage::calculateControlSignals(uint64_t mStat, uint64_t wStat)
{
	return (mStat == SADR || mStat == SINS || mStat == SHLT) || 
	(wStat == SADR || wStat == SINS || wStat == SHLT); 
}

