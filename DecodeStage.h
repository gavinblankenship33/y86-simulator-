//class to perform the combinational logic of
//the Decode Stage
class DecodeStage: public Stage
{
	private:
		uint64_t srcA;
		uint64_t srcB;
		bool ebubble;		


	public:
		bool doClockLow(PipeReg ** pregs, Stage ** stages);
		void doClockHigh(PipeReg ** pregs);
		void normal(PipeReg ** pregs);
		void bubble(PipeReg ** pregs);
		void setEInput(E * ereg, uint64_t stat, uint64_t icode, uint64_t ifun, uint64_t ValC, uint64_t ValA, uint64_t ValB, uint64_t dstE, uint64_t dstM, uint64_t srcA, uint64_t srcB);
		uint64_t d_srcA(uint64_t rA, uint64_t icode);
		uint64_t d_srcB(uint64_t rB, uint64_t icode);
		uint64_t d_dstE(uint64_t rB, uint64_t icode);
		uint64_t d_dstM(uint64_t rA, uint64_t icode);
		uint64_t d_valA(PipeReg ** pregs, Stage ** stages, uint64_t icode, uint64_t srcA, uint64_t rvalA, uint64_t valP);
		uint64_t d_valB(PipeReg ** pregs, Stage ** stages, uint64_t icode, uint64_t srcB, uint64_t rvalB);
		uint64_t getdsrcA();
		uint64_t getdsrcB();
		bool calculateControlSignals(uint64_t eicode, uint64_t edstm, uint64_t dsrca, uint64_t dsrcb, uint64_t cnd);
};


