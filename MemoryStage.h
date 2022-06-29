//class to perform the combinational logic of
//the Memory Stage
class MemoryStage: public Stage
{
	private:
		uint64_t stat;
		uint64_t valM;
		uint64_t dstM;
	public:
		bool doClockLow(PipeReg ** pregs, Stage ** stages);
		void doClockHigh(PipeReg ** pregs);
		void setWInput(W * wreg, uint64_t stat, uint64_t icode, uint64_t ValE, uint64_t ValM, uint64_t dstE, uint64_t dstM);
		uint64_t mem_addr(uint64_t icode, uint64_t valA, uint64_t valE);
		bool mem_read(uint64_t icode);
		bool mem_write(uint64_t icode);
		uint64_t getvalM();
		uint64_t getstat();
		uint64_t getdstM();
};
