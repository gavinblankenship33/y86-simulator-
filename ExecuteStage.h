//class to perform the combinational logic of
//the Execute Stage
class ExecuteStage: public Stage
{
	private:
		uint64_t dstE;
		uint64_t valE;
		uint64_t cnd;
		bool mbubble;
		
   	public:
		bool doClockLow(PipeReg ** pregs, Stage ** stages);
		void doClockHigh(PipeReg ** pregs);
		void normal(PipeReg ** pregs);
		void bubble(PipeReg ** pregs);
		void setMInput(M * mreg, uint64_t stat, uint64_t icode, uint64_t cnd, uint64_t ValE, uint64_t ValA, uint64_t dstE, uint64_t dstM);
		uint64_t aluA(uint64_t icode, uint64_t valA, uint64_t valC);
		uint64_t aluB(uint64_t icode, uint64_t valB);
		uint64_t alufun(uint64_t icode, uint64_t ifun);
		bool set_cc(E * ereg, uint64_t mStat, uint64_t wStat);
		uint64_t e_dstE(uint64_t icode, uint64_t dstE, uint64_t cnd); 
		uint64_t alu(uint64_t alufunResult, uint64_t aluAResult, uint64_t aluBResult, bool setCC);		
		uint64_t cc(uint64_t icode, uint64_t ifun);
		uint64_t getdstE();
		uint64_t getvalE();
		uint64_t get_ecnd();
		bool calculateControlSignals(uint64_t mStat, uint64_t wStat);
};
