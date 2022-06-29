//class to perform the combinational logic of
//the Fetch stage
class FetchStage: public Stage
{
   private:
      void setDInput(D * dreg, uint64_t stat, uint64_t icode, uint64_t ifun, 
                     uint64_t rA, uint64_t rB,
                     uint64_t valC, uint64_t valP);
	bool fstall;
	bool dstall;
	bool dbubble;
   public:
	bool doClockLow(PipeReg ** pregs, Stage ** stages);
	void bubble(PipeReg ** pregs);
	void normal(PipeReg ** pregs);
	void doClockHigh(PipeReg ** pregs);
	bool needRegIds(uint64_t icode);
	bool needValC(uint64_t icode);
	uint64_t selectPC(F * freg, M * mreg, W * wreg);
	uint64_t predictPC(uint64_t icode, uint64_t valC, uint64_t valP);
	void getRegIds(uint64_t &rA, uint64_t &rB, uint64_t pc);	
	uint64_t buildValC(uint64_t pc, bool needRegs);
	uint64_t PCIncrement(uint64_t pc, bool needRegs, bool needVC);
	bool instr_valid(uint64_t icode);
	uint64_t f_stat(bool error, bool instr_valid, uint64_t icode);
	uint64_t f_icode(bool error, uint64_t icode);
	uint64_t f_ifun(bool error, uint64_t ifun);
	bool f_stall(uint64_t Dicode, uint64_t Micode, uint64_t Eicode, uint64_t EdstM, uint64_t dSrcA, uint64_t dSrcB);
	bool d_stall(uint64_t Eicode, uint64_t EdstM, uint64_t dSrcA, uint64_t dSrcB);
	bool d_bubble(uint64_t Dicode,uint64_t Micode, bool e_cnd, uint64_t Eicode, uint64_t dSrcA, uint64_t dSrcB, uint64_t EdstM);
	void calculateControlSignals(PipeReg ** pregs, Stage ** stages);
};
