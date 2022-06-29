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
#include "FetchStage.h"
#include "DecodeStage.h"
#include "ExecuteStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "Memory.h"
#include "Tools.h"


/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool FetchStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   	F * freg = (F *) pregs[FREG];
   	D * dreg = (D *) pregs[DREG];
   	M * mreg = (M *) pregs[MREG];
	W * wreg = (W *) pregs[WREG];	

	Memory * mem = Memory::getInstance();
	bool error = false;

	uint64_t rA = RNONE;
	uint64_t rB = RNONE;
	uint64_t stat = SAOK;
	uint64_t valC = 0;

	uint64_t f_pc = selectPC(freg, mreg, wreg);
	uint64_t icode = Tools::getBits(mem->getByte(f_pc, error), 4, 7);
	uint64_t ifun = Tools::getBits(mem->getByte(f_pc, error), 0, 3);
	
	icode = f_icode(error, icode);
	ifun = f_ifun(error, ifun);

	bool needRegs = needRegIds(icode);
	bool needVC = needValC(icode);

	if (needVC) valC = buildValC(f_pc, needRegs);
	if (needRegs) getRegIds(rA, rB, f_pc);

	bool valid = instr_valid(icode);
        stat = f_stat(error, valid, icode);

	uint64_t valP = PCIncrement(f_pc, needRegs, needVC);
 	uint64_t pred = predictPC(icode, valC, valP);
	freg->getpredPC()->setInput(pred);

	calculateControlSignals(pregs, stages);
   	
	setDInput(dreg, stat, icode, ifun, rA, rB, valC, valP);
   	return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void FetchStage::doClockHigh(PipeReg ** pregs)
{
	if(!fstall)
	{
		F * freg = (F *) pregs[FREG];
		freg->getpredPC()->normal();
	}
	if(dbubble)
		bubble(pregs);
	else if(!dstall)
		normal(pregs);
}

void FetchStage::bubble(PipeReg ** pregs)
{
	D * dreg = (D *) pregs[DREG];
	dreg->getstat()->bubble(SAOK);
        dreg->geticode()->bubble(INOP);
        dreg->getifun()->bubble();
        dreg->getrA()->bubble(RNONE);
        dreg->getrB()->bubble(RNONE);
        dreg->getvalC()->bubble();
        dreg->getvalP()->bubble();
}

void FetchStage::normal(PipeReg ** pregs)
{
	D * dreg = (D *) pregs[DREG];
	dreg->getstat()->normal();
        dreg->geticode()->normal();
        dreg->getifun()->normal();
        dreg->getrA()->normal();
        dreg->getrB()->normal();
        dreg->getvalC()->normal();
        dreg->getvalP()->normal();
}

/* setDInput
 * provides the input to potentially be stored in the D register
 * during doClockHigh
 *
 * @param: dreg - pointer to the D register instance
 * @param: stat - value to be stored in the stat pipeline register within D
 * @param: icode - value to be stored in the icode pipeline register within D
 * @param: ifun - value to be stored in the ifun pipeline register within D
 * @param: rA - value to be stored in the rA pipeline register within D
 * @param: rB - value to be stored in the rB pipeline register within D
 * @param: valC - value to be stored in the valC pipeline register within D
 * @param: valP - value to be stored in the valP pipeline register within D
*/
void FetchStage::setDInput(D * dreg, uint64_t stat, uint64_t icode, uint64_t ifun, uint64_t rA, uint64_t rB, uint64_t valC, uint64_t valP)
{
   	dreg->getstat()->setInput(stat);
   	dreg->geticode()->setInput(icode);
   	dreg->getifun()->setInput(ifun);
   	dreg->getrA()->setInput(rA);
   	dreg->getrB()->setInput(rB);
   	dreg->getvalC()->setInput(valC);
	dreg->getvalP()->setInput(valP);
}
     
uint64_t FetchStage::selectPC(F * freg, M * mreg, W * wreg)
{
	uint64_t micode = mreg->geticode()->getOutput();
	uint64_t mcnd = mreg->getCnd()->getOutput();
	uint64_t wicode = wreg->geticode()->getOutput();

	if (micode == IJXX && !mcnd)
		return mreg->getvalA()->getOutput();
	if (wicode == IRET)
		return wreg->getvalM()->getOutput();
	else
		return freg->getpredPC()->getOutput();
}

bool FetchStage::needRegIds(uint64_t icode)
{
	return (icode == IRRMOVQ || icode == IOPQ || icode == IPUSHQ || 
	icode == IPOPQ || icode == IIRMOVQ || icode == IRMMOVQ || 
	icode == IMRMOVQ);
}

bool FetchStage::needValC(uint64_t icode)
{
	return (icode == IIRMOVQ || icode == IRMMOVQ || icode == IMRMOVQ || 
	icode == IJXX || icode == ICALL);
}

uint64_t FetchStage::predictPC(uint64_t icode, uint64_t valC, uint64_t valP)
{
	if (icode == IJXX || icode == ICALL) return valC;
	return valP;
}

void FetchStage::getRegIds(uint64_t &rA, uint64_t &rB, uint64_t pc)
{
	Memory * mem = Memory::getInstance();
	bool error;
	
	uint64_t val = mem->getByte(pc + 1, error);
	rA = Tools::getBits(val, 4, 7);
	rB = Tools::getBits(val, 0, 3);
}

uint64_t FetchStage::buildValC(uint64_t pc, bool needRegs)
{
	Memory * mem = Memory::getInstance();
        bool error;
        
	pc += 1;
	if(needRegs)
        	pc += 1;

        uint8_t valC[8];
        for(int i = 0; i < 8; i++)
        	valC[i] = mem -> getByte(pc + i, error);
        
	return Tools::buildLong(valC);
}

uint64_t FetchStage::PCIncrement(uint64_t pc, bool needRegs, bool needVC)
{
	pc += 1;
	if (needRegs) pc += 1;
	if (needVC) pc += 8;
	return pc;
}

bool FetchStage::instr_valid(uint64_t icode)
{
	return (icode == INOP || icode == IHALT || icode == IRRMOVQ || 
		icode == IIRMOVQ || icode == IRMMOVQ || icode == IMRMOVQ || 
		icode == IOPQ || icode == IJXX || icode == ICALL || icode == IRET || 
		icode == IPUSHQ || icode == IPOPQ);
}

uint64_t FetchStage::f_stat(bool error, bool instr_valid, uint64_t icode)
{
	if (error) return SADR;
	else if (!instr_valid) return SINS;
	else if (icode == IHALT) return SHLT;
	else return SAOK;
}

uint64_t FetchStage::f_icode(bool error, uint64_t icode)
{
	if (error) return INOP;
	return icode;
}

uint64_t FetchStage::f_ifun(bool error, uint64_t ifun)
{
	if (error) return FNONE;
	return ifun;
}

bool FetchStage::f_stall(uint64_t Dicode, uint64_t Micode, uint64_t Eicode, uint64_t EdstM, uint64_t dSrcA, uint64_t dSrcB)
{
	return ((Eicode == IMRMOVQ || Eicode == IPOPQ) && 
		(EdstM == dSrcA || EdstM == dSrcB)) || 
		(Dicode == IRET || Eicode == IRET || Micode == IRET);
}

bool FetchStage::d_stall(uint64_t Eicode, uint64_t EdstM, uint64_t dSrcA, uint64_t dSrcB)
{
	return ((Eicode == IMRMOVQ || Eicode == IPOPQ) && 
		(EdstM == dSrcA || EdstM == dSrcB));
}

bool FetchStage::d_bubble(uint64_t Dicode,uint64_t Micode, bool e_cnd, uint64_t Eicode, uint64_t dSrcA, uint64_t dSrcB, uint64_t EdstM)
{
	return (Eicode == IJXX && !e_cnd) || 
		(!((Eicode == IMRMOVQ || Eicode == IPOPQ) && 
		(EdstM == dSrcA || EdstM == dSrcB)) && 
		(Dicode == IRET || Eicode == IRET || Micode == IRET));
}

void FetchStage::calculateControlSignals(PipeReg ** pregs, Stage ** stages)
{
	E * ereg = (E *) pregs[EREG];
        D * dreg = (D *) pregs[DREG];
        M * mreg = (M *) pregs[MREG];
       	
	DecodeStage * ds = (DecodeStage *) stages[DSTAGE];
	ExecuteStage * es = (ExecuteStage *) stages[ESTAGE];

	uint64_t Eicode = ereg->geticode()->getOutput();
	uint64_t Micode = mreg->geticode()->getOutput();
	uint64_t Dicode = dreg->geticode()->getOutput();
	
	uint64_t EdstM = ereg->getdstM()->getOutput();
	uint64_t DsrcA = ds->getdsrcA();
	uint64_t DsrcB = ds->getdsrcB();
	uint64_t cnd = es->get_ecnd();
	
	fstall = f_stall(Dicode, Micode, Eicode, EdstM, DsrcA, DsrcB);
	dstall = d_stall(Eicode, EdstM, DsrcA, DsrcB);
	dbubble = d_bubble(Dicode, Micode, cnd, Eicode, DsrcA, DsrcB, EdstM);	
}
