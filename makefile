CC = g++
CFLAGS = -g -c -Wall -std=c++11 -O0
OBJ = yess.o Memory.o Loader.o RegisterFile.o ConditionCodes.o PipeReg.o PipeRegField.o Simulate.o F.o D.o E.o M.o W.o FetchStage.o DecodeStage.o ExecuteStage.o MemoryStage.o WritebackStage.o Tools.o

.C.o:
	$(CC) $(CFLAGS) $< -o $@

yess: $(OBJ)

yess.o: Loader.h Memory.h RegisterFile.h ConditionCodes.h Debug.h PipeReg.h Stage.h Simulate.h

Loader.o: Loader.h

Memory.o: Memory.h Tools.h

Tools.o: Tools.h

RegisterFile.o: RegisterFile.h

ConditionCodes.o: ConditionCodes.h

PipeReg.o: PipeReg.h

Simulate.o: PipeRegField.h PipeReg.h F.h D.h E.h M.h W.h Stage.h ExecuteStage.h MemoryStage.h DecodeStage.h FetchStage.h WritebackStage.h Simulate.h Memory.h RegisterFile.h ConditionCodes.h

D.o: Instructions.h RegisterFile.h PipeReg.h PipeRegField.h D.h Status.h

E.o: Instructions.h RegisterFile.h PipeReg.h PipeRegField.h E.h Status.h

F.o: PipeRegField.h PipeReg.h F.h

FetchStage.o: RegisterFile.h PipeRegField.h PipeReg.h Stage.h Status.h Debug.h F.h D.h E.h M.h W.h FetchStage.h DecodeStage.h ExecuteStage.h Instructions.h Memory.h Tools.h

DecodeStage.o: RegisterFile.h PipeRegField.h PipeReg.h Stage.h Status.h Debug.h F.h D.h E.h M.h W.h DecodeStage.h Instructions.h Tools.h ExecuteStage.h MemoryStage.h

ExecuteStage.o: RegisterFile.h PipeRegField.h PipeReg.h Stage.h Status.h Debug.h F.h D.h E.h M.h W.h ConditionCodes.h ExecuteStage.h Instructions.h Tools.h MemoryStage.h

MemoryStage.o: RegisterFile.h PipeRegField.h PipeReg.h Stage.h Status.h Debug.h F.h D.h M.h W.h Memory.h MemoryStage.h Instructions.h

WritebackStage.o: RegisterFile.h PipeRegField.h PipeReg.h Stage.h Status.h Debug.h F.h D.h M.h W.h WritebackStage.h

M.o: Instructions.h RegisterFile.h PipeReg.h PipeRegField.h M.h Status.h

PipeRegField.o: PipeRegField.h

W.o: Instructions.h RegisterFile.h PipeReg.h PipeRegField.h W.h Status.h

clean:
	rm $(OBJ) yess

run:
	make clean
	make yess
	./run.sh
	
