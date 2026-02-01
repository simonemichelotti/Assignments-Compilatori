#include "LoopWalk.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/PassManager.h"


using namespace llvm;


bool isLoopInvariant(Instruction &inst, std::set <Instruction*> set, Loop &loop){

  //Escludi istruzioni di controllo di flusso e PHI
  if (inst.getOpcode() == Instruction::Br || inst.getOpcode() == Instruction::ICmp || isa<PHINode>(inst))
    return false;
  
  //Per ogni operando
  for(auto op = inst.op_begin(); op != inst.op_end(); ++op){

    //Se l'operando è un'istruzione
    if( Instruction *op_inst = dyn_cast<Instruction>(*op) ){
      //Se l'istruzione dell'operandoappartiene al loop
      if ( loop.contains(op_inst->getParent()) ) {

        //Se non è nel set delle loop-invariant
        if(!set.empty()){
          if ( !set.count(op_inst) )
            return false;
        }
        else
          return false;
      }
      //Se l'operando è un Phi-node
      if ( isa<PHINode>(op_inst) )
        return false;
    }
    //Se l'operando è una costante
    else if ( auto *constant = dyn_cast<ConstantInt>(*op) ){
      continue;
    }
  }
  return true;
}

bool isDeadOutsideLoop(Loop &L, Instruction* Inst){

	for(auto userInst = Inst->user_begin(); userInst != Inst->user_end(); ++userInst){
		if (auto *casted = dyn_cast<Instruction>(*userInst)){ 
			Instruction &user_instruction = *casted;
			if ( !L.contains( user_instruction.getParent() ) )
        return false;
		}
	}
  
	return true;
}

bool dominatesAllLoopExits(DominatorTree &DT, Loop &L, Instruction* Inst){
  SmallVector<BasicBlock*> ExitBlocks;
  //Ottieni tutti gli Exit Blocks del loop
  L.getExitBlocks(ExitBlocks);

  //Ottieni il Basic Block dell'istruzione
  BasicBlock *instructionParentBB = Inst->getParent();

  //Verifica che il Basic Block domini tutti gli Exit Blocks
  for (auto *ExitBlock : ExitBlocks) {
    if(!DT.dominates(instructionParentBB, ExitBlock))
      return false;
  }
  return true;
}

bool dominatesAllUses(DominatorTree &DT, Instruction* Inst) {
  //Ottieni il Basic Block in cui è definita l'istruzione
  BasicBlock *defBB = Inst->getParent();
  //Per ogni uso dell'istruzione
  for (auto *user : Inst->users()) {
    //Verifica che il Basic Block di definizione domini il Basic Block di ogni uso
    if (auto *userInst = dyn_cast<Instruction>(user)) {
      BasicBlock *useBB = userInst->getParent();
      if (!DT.dominates(defBB, useBB)) {
        return false;
      }
    }
  }
  return true;
}

bool isOnlyDefinitionInLoop(Loop &L, Instruction* Inst) {
    Value* definedValue = Inst;
    for (auto BI = L.block_begin(); BI != L.block_end(); ++BI) {
        for (auto II = (*BI)->begin(); II != (*BI)->end(); ++II) {
            Instruction* otherInst = &*II;
            // Salta se è la stessa istruzione
            if (otherInst == Inst) continue;
            // Se c'è un'altra istruzione che definisce la stessa variabile
            if (otherInst->getName() == Inst->getName() && !otherInst->getName().empty())
                return false;
        }
    }
    return true;
}

bool operandsAlreadyMoved(Instruction* Inst, std::set <Instruction*> set){ 
	//Per ogni operando dell'istruzione
  for(auto op = Inst->op_begin(); op != Inst->op_end(); ++op){
    //Se l'operando è un'istruzione
    if( Instruction *op_inst = dyn_cast<Instruction>(*op) ){
      //Verifica che l'istruzione dell'operando sia già stata spostata
      if (set.count(op_inst))
        return false;
    }
  }
  return true;
}

PreservedAnalyses LoopWalk::run(Loop &L, LoopAnalysisManager &LAM, LoopStandardAnalysisResults &LAR, LPMUpdater &LU) {

  //Crea un set per tenere traccia delle istruzioni loop-invariant
  std::set <Instruction*> loopInvariantInstructionsSet;

  //Verifica che il loop sia in forma normale
  if(!L.isLoopSimplifyForm()) {
  	outs() << "Loop not in Normal Form \n";
    return PreservedAnalyses::all();
  }

   //Per ogni BB per ogni istruzione, verifica se è loop-invariant
  for (Loop::block_iterator BI = L.block_begin(); BI != L.block_end(); ++BI){       
    BasicBlock *BB = *BI;
    for (auto inst = BB->begin(); inst != BB->end(); ++inst ){                      
    Instruction &Inst = *inst;
    if ( isLoopInvariant(Inst, loopInvariantInstructionsSet, L) &&
         dominatesAllUses(LAR.DT, &Inst) &&
         isOnlyDefinitionInLoop(L, &Inst) &&
         (isDeadOutsideLoop(L, &Inst) || dominatesAllLoopExits(LAR.DT, L, &Inst)) ) {
        loopInvariantInstructionsSet.insert(&Inst);
    }
  }
}
  //Ottieni il preheader del loop
  BasicBlock* LoopPreheader = L.getLoopPreheader();
  //Punto di inserimento: prima del terminatore del preheader
  Instruction *insertPoint = LoopPreheader->getTerminator();

  // Sposta le istruzioni loop-invariant nel preheader
while (!loopInvariantInstructionsSet.empty()) {
    std::vector<Instruction*> toMove;
    for (Instruction* Inst : loopInvariantInstructionsSet) {
        // L'istruzione viene spostata fuori dal loop solo dopo che tutti i suoi operandi sono già stati spostati
        if (operandsAlreadyMoved(Inst, loopInvariantInstructionsSet)) {
            toMove.push_back(Inst);
        }
    }
    // Se non ci sono istruzioni da spostare, esci per evitare loop infinito
    if (toMove.empty()) break;
    for (Instruction* Inst : toMove) {
        outs() << "Moving --> " << *Inst << "\n";
        Inst->removeFromParent();
        Inst->insertBefore(insertPoint);
        loopInvariantInstructionsSet.erase(Inst);
    }
}

  return PreservedAnalyses::all();
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "LoopWalk", LLVM_VERSION_STRING,
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, LoopPassManager &LPM,
           ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "LoopWalk") {
            LPM.addPass(LoopWalk());
            return true;
          }
          return false;
        });
    }
  };
}
